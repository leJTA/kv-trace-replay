#include <fstream>
#include <iostream>
#include <string_view>

#include "httplib.h"

#include "trace_player.hpp"

int main(int argc, char* argv[])
{
	Config config;
	config.parse_command_line(argc, argv);

	if (config.error) {
		std::cout << config.error_msg << "\n";
		return EXIT_FAILURE;
	}

	if (config.help) {
		std::cout << config.help_msg << "\n";
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

	KV_trace::Trace_player player{config.trace_file, config.data_file, config.host,
								  config.port,		 config.nthreads,  config.buffer_size};

	std::error_code ec = player.run();
	if (ec) {
		std::cout << ec;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}