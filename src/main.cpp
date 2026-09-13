#include <fstream>
#include <print>
#include <string_view>

#include "httplib.h"

#include "trace_player.hpp"

int main(int argc, char* argv[])
{
	Config config;
	config.parse_command_line(argc, argv);

	if (config.error) {
		std::println(stderr, "{}", config.error_msg);
		return EXIT_FAILURE;
	}

	if (config.help) {
		std::print("{}", config.help_msg);
		return EXIT_SUCCESS;
	}

	// std::cout << "[INFO] ----------------------------------------------------------------\n";
	// std::cout << "[INFO] Key-Value Trace Player\n";
	// std::cout << "[INFO] ----------------------------------------------------------------\n";
	// std::cout << "[INFO] Trace file path     : " << config.trace_file << "\n";
	// std::cout << "[INFO] Data file path      : " << config.data_file << "\n";
	// std::cout << "[INFO] Host & port         : " << config.host << ":" << config.port << "\n";
	// std::cout << "[INFO] Threads             : " << config.nthreads << "\n";
	// std::cout << "[INFO] Request Buffer size : " << config.buffer_size << "\n";
	// std::cout << "[INFO] ----------------------------------------------------------------\n\n";

	KV_trace::Trace_player player{config};

	std::error_code ec = player.run();
	if (ec) {
		std::cout << ec.message() << "\n";
		return EXIT_FAILURE;
	}

	std::println(
		"total requests = {}, min = {} ms, max = {} ms, avg = {} ms, median = {} ms, p90 = {} ms, "
		"p95 = {} ms, p99 = {} ms, p99.9 = {} ms",
		player.statistics()->total_count(), player.statistics()->min() / 1000.0,
		player.statistics()->max() / 1000.0, player.statistics()->mean() / 1000.0,
		player.statistics()->percentile(50) / 1000.0, player.statistics()->percentile(90) / 1000.0,
		player.statistics()->percentile(95) / 1000.0, player.statistics()->percentile(99) / 1000.0,
		player.statistics()->percentile(99.9) / 1000.0);

	return EXIT_SUCCESS;
}