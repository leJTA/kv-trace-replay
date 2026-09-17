#include <print>
#include <iostream>
#include <string>

#include <boost/program_options.hpp>

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

	// TODO:
	// 1. Open the RocksDB database.
	// 2. Read the trace.
	// 3. Generate/read the value corresponding to each request.
	// 4. Populate RocksDB.

	return EXIT_SUCCESS;
}