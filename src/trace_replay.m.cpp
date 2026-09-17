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
	std::println("Host & port         : {}", config.host);
	std::println("Threads             : {}", config.nthreads);
	std::println("Request Buffer size : {}", config.buffer_size);
	std::println("-----------------------------------------------------------------");

	KV_trace::Trace_player player{config};

	std::error_code ec = player.run();
	if (ec) {
		std::println("{}", ec.message());
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