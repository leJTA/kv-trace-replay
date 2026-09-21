#include "config.hpp"

#include <boost/program_options.hpp>
#include <iostream>

namespace KV_trace {
	std::istream& operator>>(std::istream& is, Protocol& protocol)
	{
		std::string token;
		is >> token;

		if (token == "http")
			protocol = Protocol::http;
		else if (token == "tcp")
			protocol = KV_trace::Protocol::tcp;
		else if (token == "console")
			protocol = Protocol::console;
		else
			is.setstate(std::ios_base::failbit);

		return is;
	}

	std::ostream& operator<<(std::ostream& os, Protocol protocol)
	{
		switch (protocol) {
		case Protocol::http:
			return os << "http";
		case Protocol::tcp:
			return os << "tcp";
		case Protocol::console:
			return os << "console";
		}

		return os;
	}

	void Config::parse_command_line(int argc, const char* const argv[])
	{
		namespace po = boost::program_options;
		try {
			po::options_description desc{"Command line options"};
			// clang-format off
			desc.add_options()
				("help", "print this help message")
				("trace-file,f", po::value<std::string>()->required(), "trace file")
				("data-file,d", po::value<std::string>()->required(), "data source file")
				("host,h", po::value<std::string>()->default_value("localhost"), "host address")
				("ignore-timing", po::bool_switch(&ignore_timing),
					"execute trace requests without respecting their timestamps")
				("port,p", po::value<int>()->default_value(9000), "host port")
				("protocol,x", po::value<Protocol>(&protocol)->default_value(Protocol::http),
					"request protocol: http, tcp or console")
				("threads,t", po::value<int>()->default_value(4), "number of sender threads")
				("buffer-size,b", po::value<size_t>()->default_value(4096), "size of the request buffer")
				("output-csv,o", po::value<std::string>(), "export the statistics to the given csv file")
				("cdf", po::value<std::string>(), "export the CDF to the given file")
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
			if (vm.count("output-csv")) {
				output_csv = vm["output-csv"].as<std::string>();
			}
			if (vm.count("cdf")) {
				output_cdf = vm["cdf"].as<std::string>();
			}
		}
		catch (std::exception& e) {
			error = true;
			error_msg = e.what();
		}
	}
} // namespace KV_trace
