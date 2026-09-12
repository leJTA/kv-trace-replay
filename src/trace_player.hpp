#ifndef __TRACE_PLAYER_HPP__
#define __TRACE_PLAYER_HPP__

#include <chrono>
#include <memory>
#include <print>
#include <string_view>

#include <libmemcached/memcached.h>

#include "client_pool.hpp"
#include "config.hpp"
#include "data_source.hpp"
#include "request.hpp"
#include "trace_error.hpp"
#include "trace_producer.hpp"

static constexpr size_t _max_data_size = 256 * 1024; // 256 KB

namespace KV_trace {
	class Trace_player {
	public:
		explicit Trace_player(const Config& config)
			: _host{config.host}, _port{config.port}, _data_file{config.data_file},
			  _data_source{_max_data_size}, _request_buffer{config.buffer_size},
			  _trace_producer{config.trace_file}, _client_pool{config.nthreads}
		{}

		std::error_code run()
		{
			std::error_code ec;
			ec = _data_source.load(_data_file);
			if (ec) {
				return ec;
			}

			_trace_producer.attach_request_buffer(_request_buffer);
			_client_pool.attach_request_buffer(_request_buffer);

			_client_pool.set_request_handler(
				[this](const Request& r) { console_request_handler(r); });
			_client_pool.start();

			ec = _trace_producer.start();
			if (ec) {
				_request_buffer.close();
				return ec;
			}

			// close the queue to notify clients so that they do not get stuck waiting for new
			// requests
			_request_buffer.close();
			return {};
		}

	private:
		std::string _host;
		int _port;
		std::string _data_file;
		Data_source _data_source;
		Request_buffer _request_buffer;
		Trace_producer _trace_producer;
		Client_pool _client_pool;

		void console_request_handler(const Request& req)
		{
			// wait until the time to send the request arrives
			std::this_thread::sleep_until(_client_pool.start_time +
										  std::chrono::seconds(req.timestamp));
			
										  // send request
			if (req.operation == Operation::op_get) {
				std::println("[{}] GET {}", req.timestamp, req.key);
			}
			else if (req.operation == Operation::op_set) {
				std::println("[{}] SET {} [size = {}, ttl = {}]", req.timestamp, req.key,
							 req.value_size, req.ttl);
			}
		}

		void http_request_handler(const Request& req)
		{
			thread_local httplib::Client http_client{_host, _port};
			// wait until the time to send the request arrives
			std::this_thread::sleep_until(_client_pool.start_time +
										  std::chrono::seconds(req.timestamp));

			// send request
			if (req.operation == Operation::op_get) {
				http_client.Get(std::string("/").append(req.key));
			}
			else if (req.operation == Operation::op_set) {
				http_client.Post(std::string("/").append(req.key), _data_source.data(),
								 req.value_size, "application/octet-stream");
			}
		}

		// void memcached_request_handler(const Request& req)
		// {
		// 	thread_local auto memc = std::unique_ptr<memcached_st>(memcached_create(nullptr));
		// 	memcached_server_add(memc.get(), _host.c_str(), _port);
		// 	size_t len = req.value_size;
		// 	uint32_t flags;
		// 	if (req.operation == Operation::op_get) {
		// 		char* val = memcached_get(memc.get(), req.key, req.key_size, &len, &flags, NULL);
		// 	}
		// 	else if (req.operation == Operation::op_set) {
		// 		memcached_set(memc.get(), req.key, req.key_size, _data_source.data(),
		// 					  req.value_size, 0, 0);
		// 	}
		// }
	};
} // namespace KV_trace

#endif // __TRACE_PLAYER_HPP__