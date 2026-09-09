#include <fstream>
#include <iostream>

#include "config.hpp"
#include "request.hpp"

int main(int argc, char* argv[])
{
	Config config;
	config.parse_command_line(argc, argv);

	if (config.help) {
		std::cout << config.help_msg << "\n";
		return EXIT_SUCCESS;
	}

	if (config.error) {
		std::cout << config.error_msg << "\n";
		return EXIT_FAILURE;
	}

	std::ifstream trace_file{config.trace_file};
	Request_buffer request_buffer{config.buffer_size};
	std::string line;

	if (!trace_file.is_open()) {
		std::cerr << "Error opening trace file : " << config.trace_file << "\n";
	}

	while (std::getline(trace_file, line)) {
		Request request = Request::parse_line(line);
		request_buffer.push(request);
		// TODO : sender pool
	}
	trace_file.close();

	return EXIT_SUCCESS;
}