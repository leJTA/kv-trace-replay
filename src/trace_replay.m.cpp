#include <fstream>
#include <print>
#include <string_view>

#include "trace_player.hpp"

int main(int argc, char* argv[])
{
	KV_trace::Config config;
	config.parse_command_line(argc, argv);

	if (config.error) {
		std::println(stderr, "{}", config.error_msg);
		return EXIT_FAILURE;
	}

	if (config.help) {
		std::print("{}", config.help_msg);
		return EXIT_SUCCESS;
	}

	std::println("----------------------------------------------------------------");
	std::println("Trace file path     : {}", config.trace_file);
	std::println("Data file path      : {}", config.data_file);
	std::println("Host & port         : {}:{}", config.host, config.port);
	// std::println("Protocol            : {}", config.protocol);
	std::println("Threads             : {}", config.nthreads);
	std::println("Request Buffer size : {}", config.buffer_size);
	std::println("-----------------------------------------------------------------");

	KV_trace::Trace_player player{config};

	std::error_code ec = player.run();
	if (ec) {
		std::println("{}", ec.message());
		return EXIT_FAILURE;
	}

	constexpr double thounsand = 1'000.0;
	const auto* stats = player.statistics();
	std::println(
		"requests = {}, time = {} ms, min = {} ms, max = {} ms, avg = {} ms, median = {} ms, "
		"p90 = {} ms, p95 = {} ms, p99 = {} ms, p99.9 = {} ms",
		stats->total_count(), stats->total_time() / thounsand, stats->min() / thounsand,
		stats->max() / thounsand, stats->mean() / thounsand, stats->percentile(50) / thounsand,
		stats->percentile(90) / thounsand, stats->percentile(95) / thounsand,
		stats->percentile(99) / thounsand, stats->percentile(99.9) / thounsand);

	return EXIT_SUCCESS;
}