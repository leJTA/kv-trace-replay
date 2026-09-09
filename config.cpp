#include "config.hpp"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

void Config::parse_command_line(int argc, const char* const argv[])
{
	try {
		po::options_description desc{"Command line options"};
		// clang-format off
        desc.add_options()
            ("help", "print this help message")
            ("trace-file,f", po::value<std::string>()->required(), "trace file")
            ("data-file,d", po::value<std::string>()->required(), "data source file")
            ("host,h", po::value<std::string>()->default_value("localhost"), "host address")
            ("port,p", po::value<int>()->default_value(9000), "host port")
            ("threads,t", po::value<int>()->default_value(4), "number of worker threads")
            ("buffer-size", po::value<size_t>()->default_value(4096), "size of the request buffer")
        ;
		// clang-format on
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);

		if (vm.count("help")) {
			help = true;
			std::stringstream ss;
			ss << desc;
			help_msg = std::move(ss).str();
			return;
		}

		po::notify(vm);

		trace_file = vm["trace-file"].as<std::string>();
		data_file = vm["data-file"].as<std::string>();
		host = vm["host"].as<std::string>();
		port = vm["port"].as<int>();
		nthreads = vm["threads"].as<int>();
		buffer_size = vm["buffer-size"].as<size_t>();
	}
	catch (std::exception& e) {
		error = true;
		error_msg = e.what();
	}
}