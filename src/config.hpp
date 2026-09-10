#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <string>

struct Config {
	void parse_command_line(int argc, const char* const argv[]);

	bool help = false;
	bool error = false;
	std::string error_msg;
	std::string help_msg;
	std::string trace_file;
	std::string data_file;
	std::string host;
	int port;
	int nthreads;
	size_t buffer_size;
};

#endif // __CONFIG_HPP__