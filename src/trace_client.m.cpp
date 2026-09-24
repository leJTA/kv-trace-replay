#include <hdr/hdr_histogram.h>

#include <boost/program_options.hpp>
#include <httplib.h>
#include <libmemcached/memcached.h>
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/table.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <print>
#include <string>

namespace po = boost::program_options;

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

struct Config {
	uint16_t port = 0;
	std::string db_path;
	unsigned int threads = 1;
	std::string memc_host;
	uint16_t memc_port;
};

// -----------------------------------------------------------------------------
// Statistics
// -----------------------------------------------------------------------------

struct Statistics {
	std::unique_ptr<hdr_histogram, decltype(&hdr_close)> times{nullptr, &hdr_close};
	std::atomic<uint64_t> hits{0};
	std::atomic<uint64_t> misses{0};
	std::mutex histogram_mutex;

	Statistics()
	{
		hdr_histogram* time_histogram = nullptr;
		hdr_init(1, INT64_C(60'000'000'000), 3, &time_histogram);
		times.reset(time_histogram);
	}

	void record_hit() { ++hits; }
	void record_miss() { ++misses; }
	void record_time(int64_t elapsed_ns)
	{
		std::lock_guard lock{histogram_mutex};
		hdr_record_value(times.get(), elapsed_ns);
	}

	int64_t percentile(double percentile) const
	{
		return hdr_value_at_percentile(times.get(), percentile);
	};
	int64_t mean() const { return hdr_mean(times.get()); }
	int64_t min() const { return hdr_min(times.get()); }
	int64_t max() const { return hdr_max(times.get()); }
	int64_t total_count() const { return times->total_count; }
};

// -----------------------------------------------------------------------------
// Client
// -----------------------------------------------------------------------------

class Client {
public:
	Client(const Config& config, Statistics& statistics)
		: _statistics{statistics}, _memc_host{config.memc_host}, _memc_port{config.memc_port}
	{
		rocksdb::Options options;
		rocksdb::BlockBasedTableOptions table_options;
		options.create_if_missing = false;
		rocksdb::DB* db = nullptr;

		// Disable caching
		table_options.no_block_cache = true;
		table_options.cache_index_and_filter_blocks = false;
		options.table_factory.reset(NewBlockBasedTableFactory(table_options));

		options.create_if_missing = true;

		// Blob options
		options.enable_blob_files = true;
		options.enable_blob_garbage_collection = false;

		// Use Direct IO to avoid using Linux page cache
		options.use_direct_reads = true;
		options.use_direct_io_for_flush_and_compaction = true;

		// Disable compaction for stable backend
		options.disable_auto_compactions = true;

		// Write buffer size for big objects
		options.write_buffer_size = 512 * 1024 * 1024; // 512MB
		options.max_write_buffer_number = 3;

		// Limit the number of compaction
		options.target_file_size_base = 512 * 1024 * 1024; // 512MB

		// Disable compression to avoid CPU overhead
		options.compression = rocksdb::kNoCompression;

		// Open DB
		const auto status = rocksdb::DB::Open(options, config.db_path, &db);
		if (!status.ok()) {
			std::println(stderr, "failed to open RocksDB: {}", status.ToString());
		}

		_db.reset(db);
	}

	// Cache-aside GET.
	bool get(const std::string& key, std::string& value)
	{
		const auto start = std::chrono::steady_clock::now();
		memcached_st* memc = _memc_client();

		// try to get from Memcached
		size_t len;
		uint32_t flags;
		std::unique_ptr<char[]> val{
			memcached_get(memc, key.c_str(), key.size(), &len, &flags, NULL)};

		if (val.get()) { // cache hit
			value.assign(val.get(), len);
			const auto end = std::chrono::steady_clock::now();
			const auto elapsed =
				std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

			_statistics.record_hit();
			_statistics.record_time(elapsed);

			return true;
		}
		// cache miss
		const auto status = _db->Get(rocksdb::ReadOptions{}, key, &value);

		if (!status.ok()) {
			return false;
		}

		// store in Memcached
		memcached_set(memc, key.c_str(), key.size(), value.data(), value.length(), 0, 0);

		const auto end = std::chrono::steady_clock::now();
		const auto elapsed =
			std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

		_statistics.record_miss();
		_statistics.record_time(elapsed);

		return true;
	}

	bool set(const std::string& key, const std::string& value)
	{
		memcached_st* memc = _memc_client();
		const auto start = std::chrono::steady_clock::now();

		memcached_set(memc, key.c_str(), key.size(), value.data(), value.length(), 0, 0);
		const auto status = _db->Put(rocksdb::WriteOptions{}, key, value);
		if (!status.ok()) {
			return false;
		}

		const auto end = std::chrono::steady_clock::now();
		const auto elapsed =
			std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

		_statistics.record_time(elapsed);

		return true;
	}

private:
	Statistics& _statistics;
	std::unique_ptr<rocksdb::DB> _db;
	std::string _memc_host;
	uint16_t _memc_port;

	memcached_st* _memc_client()
	{
		thread_local std::unique_ptr<memcached_st, decltype(&memcached_free)> client{
			nullptr, &memcached_free};

		if (!client) {
			client.reset(memcached_create(nullptr));
			memcached_server_add(client.get(), _memc_host.c_str(), _memc_port);
			std::println("new memcached client created");
		}
		return client.get();
	}
};

// -----------------------------------------------------------------------------
// Signal handling
// -----------------------------------------------------------------------------

namespace {
	httplib::Server* server_ptr;
	void handle_signal(int sig)
	{
		if (sig == SIGINT || sig == SIGTERM) {
			std::cerr << "\n[INFO] Caught Ctrl+C, stopping server..." << std::endl;
			server_ptr->stop(); // Gracefully stop accepting new requests
		}
	}
} // namespace

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	Config config;
	po::options_description desc{"Options"};

	// clang-format off
	desc.add_options()("help,h", "print this help message")
        ("port,p", po::value<uint16_t>(&config.port)->required(), "listening port")
        ("db-path,b", po::value<std::string>(&config.db_path)->required(),"rocksDB path")
        ("threads,t", po::value<unsigned int>(&config.threads)->default_value(1),
            "number of HTTP worker threads")
        ("memc-host", po::value<std::string>(&config.memc_host)->default_value("127.0.0.1"),
            "Memcached host")
        ("memc-port", po::value<uint16_t>(&config.memc_port)->default_value(11211), 
            "Memcached port");
	// clang-format on

	try {
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);

		if (vm.count("help")) {
			std::cout << desc;
			return EXIT_SUCCESS;
		}

		po::notify(vm);
	}
	catch (const po::error& e) {
		std::println("{}", e.what());
		return EXIT_FAILURE;
	}

	Statistics statistics;
	Client client{config, statistics};
	httplib::Server server;

	server.new_task_queue = [&config] { return new httplib::ThreadPool(config.threads, 0); };

	// ---------------------------------------------------------------------
	// GET /key
	// ---------------------------------------------------------------------

	server.Get(R"(/(.+))", [&client](const httplib::Request& req, httplib::Response& res) {
		const std::string key = req.matches[1];
		thread_local std::string value;

		if (!client.get(key, value)) {
			res.status = 404;
			return;
		}

		res.set_content(value, "application/octet-stream");
		value.clear();
	});

	server.Post(R"(/(.+))", [&client](const httplib::Request& req, httplib::Response& res) {
		const std::string key = req.matches[1];

		if (!client.set(key, req.body)) {
			res.status = 404;
			return;
		}
	});

	std::println("Trace client listening on port {} with {} threads", config.port, config.threads);

	// Register signal handler
	server_ptr = &server;
	std::signal(SIGINT, handle_signal);
	std::signal(SIGTERM, handle_signal);

	if (!server.listen("0.0.0.0", config.port)) {
		std::println(stderr, "server start-up failure");
		return EXIT_FAILURE;
	}

	constexpr double million = 1'000'000.0;
	std::println("requests = {}, hits = {}, misses = {}, min = {} ms, max = {} ms, avg = {} ms, "
				 "median = {} ms, p90 = {} ms, p95 = {} ms, p99 = {} ms, p99.9 = {} ms",
				 statistics.total_count(), (ulong)statistics.hits, (ulong)statistics.misses,
				 statistics.min() / million, statistics.max() / million,
				 statistics.mean() / million, statistics.percentile(50) / million,
				 statistics.percentile(90) / million, statistics.percentile(95) / million,
				 statistics.percentile(99) / million, statistics.percentile(99.9) / million);

	return EXIT_SUCCESS;
}