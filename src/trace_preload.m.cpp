#include <iostream>
#include <print>
#include <string>
#include <system_error>

#include <boost/program_options.hpp>
#include <rocksdb/db.h>
#include <rocksdb/options.h>

#include "data_source.hpp"
#include "request.hpp"

namespace KV_trace {

	struct Config {
		std::string data_file;
		std::string trace_file;
		std::string db_path;
	};

	Config parse_command_line(int argc, const char* const argv[])
	{
		namespace po = boost::program_options;
		Config config;
		po::options_description desc{"Command line options"};

		// clang-format off
        desc.add_options()
            ("help,h","print this help message")
            ("data-file,d", po::value<std::string>(&config.data_file)->required(), 
                "data source file")
            ("trace-file,t", po::value<std::string>(&config.trace_file)->required(), "trace file")
            ("db-path,b", po::value<std::string>(&config.db_path)->required(), 
                "RocksDB database path");
		// clang-format on

		try {
			po::variables_map vm;
			po::store(po::parse_command_line(argc, argv, desc), vm);

			if (vm.count("help")) {
				std::cout << desc;
				std::exit(EXIT_SUCCESS);
			}

			po::notify(vm);
		}
		catch (const po::error& e) {
			std::println("Error: {}", e.what());
			std::exit(EXIT_FAILURE);
		}

		return config;
	}

} // namespace KV_trace

int main(int argc, char* argv[])
{
	const auto config = KV_trace::parse_command_line(argc, argv);

	std::println("Data file  : {}", config.data_file);
	std::println("Trace file : {}", config.trace_file);
	std::println("DB path    : {}", config.db_path);

	std::ifstream trace{config.trace_file};
	if (!trace) {
		std::println(stderr, "Failed to open trace file: {}", config.trace_file);
		return EXIT_FAILURE;
	}

	// First pass : find the maximum value
	std::size_t max_value_size = 0;
	std::string line;
	while (std::getline(trace, line)) {
		const auto request = KV_trace::request_from_csv_line(line);
		if (request.operation != KV_trace::Operation::op_get)
			continue;

		max_value_size = std::max<std::size_t>(max_value_size, request.value_size);
	}

	trace.close();

	if (max_value_size == 0) {
		std::println(stderr, "No GET request with a value was found in the trace");
		return EXIT_FAILURE;
	}

	// Load the data source
	KV_trace::Data_source data_source{max_value_size};
	if (const auto ec = data_source.load(config.data_file)) {
		std::println("{}", ec.message());
		return EXIT_FAILURE;
	}

	// Open RocksDB
	rocksdb::Options options;
	options.create_if_missing = true;
	rocksdb::DB* db = nullptr;

	auto status = rocksdb::DB::Open(options, config.db_path, &db);
	if (!status.ok()) {
		std::println("Failed to open RocksDB: {}", status.ToString());
		return EXIT_FAILURE;
	}

	// Second pass: populate data to RocksDB
	trace.open(config.trace_file);
	if (!trace) {
		std::println("Failed to reopen trace file: {}", config.trace_file);
		delete db;
		return EXIT_FAILURE;
	}

	std::uint64_t inserted = 0;
	std::uint64_t total_size = 0;
	while (std::getline(trace, line)) {
		if (line.empty())
			continue;

		const auto request = KV_trace::request_from_csv_line(line);
		if (request.value_size == 0)
			continue;

		const std::string key{request.key, request.key_size};
		const rocksdb::Slice value{data_source.data(), request.value_size};

		status = db->Put(rocksdb::WriteOptions{}, key, value);
		if (!status.ok()) {
			std::println("Failed to insert key '{}': {}", key, status.ToString());
			delete db;
			return EXIT_FAILURE;
		}
		std::println("key = '{}' size = {}", key, request.value_size);

		++inserted;
		total_size += request.key_size;
	}

	delete db;

	std::println("---------------------------------");
	std::println("Preload completed");
	std::println("---------------------------------");
	std::println("Maximum value size: {}", max_value_size);
	std::println("Inserted requests: {}", inserted);

	return EXIT_SUCCESS;
}