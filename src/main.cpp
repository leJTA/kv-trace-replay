#include <chrono>
#include <fstream>
#include <iostream>
#include <string_view>

#include "httplib.h"

#include "client_pool.hpp"
#include "config.hpp"
#include "data_source.hpp"
#include "request.hpp"
#include "trace_producer.hpp"

constexpr size_t max_size = 256 * 1024; // 256 KB

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

	Data_source data_source{max_size};
	if (!data_source.load(config.data_file)) {
		return EXIT_FAILURE;
	}

	Request_buffer request_buffer{config.buffer_size};
	thread_local std::unique_ptr<httplib::Client> http_client;

	Client_pool client_pool{config.nthreads};
	client_pool.attach_request_buffer(request_buffer);
	client_pool.set_request_handler([&](const Request& req) {
		// wait until the time to send the request arrives
		http_client = std::make_unique<httplib::Client>(config.host, config.port);
		std::this_thread::sleep_until(client_pool.start_time + std::chrono::seconds(req.timestamp));

		// send request
		if (req.operation == std::string_view("get")) {
			http_client->Get(std::string("/").append(req.key));
		}

		if (req.operation == std::string_view("set")) {
			http_client->Post(std::string("/").append(req.key), data_source.data(), req.value_size,
							  "application/octet-stream");
		}
	});
	client_pool.start();

	Trace_producer trace_producer{config.trace_file};
	trace_producer.attach_request_buffer(request_buffer);
	if (!trace_producer.start()) {
		std::cout << "Unable to open file: " << config.trace_file << "\n";
	}

	request_buffer.close();

	return EXIT_SUCCESS;
}