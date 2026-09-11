#ifndef __TRACE_PLAYER_HPP__
#define __TRACE_PLAYER_HPP__

#include <chrono>
#include <memory>
#include <string_view>

#include <libmemcached/memcached.h>

#include "client_pool.hpp"
#include "config.hpp"
#include "data_source.hpp"
#include "request.hpp"
#include "trace_producer.hpp"

static constexpr size_t _max_data_size = 256 * 1024; // 256 KB

namespace KV_trace {
	class Trace_player {
	public:
		Trace_player(const std::string& trace_file, const std::string& data_file,
					 const std::string& host, int port, int nthreads, size_t buffer_size)
			: _host{host}, _port{port}, _data_file{data_file}, _data_source{_max_data_size},
			  _request_buffer{buffer_size}, _trace_producer{trace_file}, _client_pool{nthreads}
		{}

		bool run()
		{
			if (!_data_source.load(_data_file)) {
				return false;
			}

			_trace_producer.attach_request_buffer(_request_buffer);
			_client_pool.attach_request_buffer(_request_buffer);

			_client_pool.set_request_handler([this](const Request& r) { HTTP_request_handler(r); });
			_client_pool.start();

			if (!_trace_producer.start()) {
				std::cout << "Unable to open trace file \n";
				return false;
			}

			// close the queue to notify clients so that they do not get stuck waiting for new
			// requests
			_request_buffer.close();
			return true;
		}

	private:
		std::string _host;
		int _port;
		std::string _data_file;
		Data_source _data_source;
		Request_buffer _request_buffer;
		Trace_producer _trace_producer;
		Client_pool _client_pool;

		void HTTP_request_handler(const Request& req)
		{
			thread_local httplib::Client http_client{_host, _port};
			// wait until the time to send the request arrives
			std::this_thread::sleep_until(_client_pool.start_time +
										  std::chrono::seconds(req.timestamp));

			// send request
			if (req.operation == Operation::OP_GET) {
				http_client.Get(std::string("/").append(req.key));
			}
			else if (req.operation == Operation::OP_SET) {
				http_client.Post(std::string("/").append(req.key), _data_source.data(),
								 req.value_size, "application/octet-stream");
			}
		}

		// void Memcached_request_handler(const Request& req)
		// {
		// 	thread_local auto memc = std::unique_ptr<memcached_st>(memcached_create(nullptr));
		// 	memcached_server_add(memc.get(), _host.c_str(), _port);
		// 	size_t len = req.value_size;
		// 	uint32_t flags;
		// 	if (req.operation == Operation::OP_GET) {
		// 		char* val = memcached_get(memc.get(), req.key, req.key_size, &len, &flags, NULL);
		// 	}
		// 	else if (req.operation == Operation::OP_SET) {
		// 		memcached_set(memc.get(), req.key, req.key_size, _data_source.data(),
		// 					  req.value_size, 0, 0);
		// 	}
		// }
	};
} // namespace KV_trace

#endif // __TRACE_PLAYER_HPP__