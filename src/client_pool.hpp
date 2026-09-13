#ifndef __CLIENT_POOL_HPP__
#define __CLIENT_POOL_HPP__

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include <vector>

#include "data_source.hpp"
#include "request.hpp"
#include "statistics.hpp"

namespace KV_trace {
	using Request_handler = std::function<void(const Request&)>;

	class Client_pool {
	public:
		std::chrono::steady_clock::time_point start_time;

		Client_pool(int pool_size, const std::string& host, int port)
			: _request_buffer{nullptr}, _pool_size{pool_size}, _host{host}, _port{port}
		{}

		void set_request_handler(Request_handler handler) { _request_handler = handler; }
		void set_data_source(Data_source* data_source) { _data_source = data_source; }
		void attach_request_buffer(Request_buffer* request_buffer)
		{
			_request_buffer = request_buffer;
		}

		void start()
		{
			start_time = std::chrono::steady_clock::now();
			for (int i = 0; i < _pool_size; ++i) {
				_pool.emplace_back([this]() {
					while (auto request = this->_request_buffer->pop()) [[likely]] {
						this->_request_handler(*request);
					}
					_total_stats.add(_stats);
				});
			}
		}

		void wait()
		{
			for (auto& client : _pool) {
				client.join();
			}
		}

		// Statistics
		Statistics* statistics() const { return &_total_stats; }

		// Request Handlers
		void console_request_handler(const Request& req)
		{
			// wait until the time to send the request arrives
			std::this_thread::sleep_until(start_time + std::chrono::seconds(req.timestamp));

			auto begin = std::chrono::steady_clock::now();
			// console print request
			if (req.operation == Operation::op_get) {
				std::println("[{}] GET {}", req.timestamp, req.key);
			}
			else if (req.operation == Operation::op_set) {
				std::println("[{}] SET {} [size = {}, ttl = {}]", req.timestamp, req.key,
							 req.value_size, req.ttl);
			}
			auto end = std::chrono::steady_clock::now();
			_stats.record_time(
				std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count());
		}

		void http_request_handler(const Request& req)
		{
			thread_local httplib::Client http_client{_host, _port};
			// wait until the time to send the request arrives
			std::this_thread::sleep_until(start_time + std::chrono::seconds(req.timestamp));

			// send http request
			if (req.operation == Operation::op_get) {
				http_client.Get(std::string("/").append(req.key));
			}
			else if (req.operation == Operation::op_set) {
				http_client.Post(std::string("/").append(req.key), _data_source->data(),
								 req.value_size, "application/octet-stream");
			}
		}

		// void memcached_request_handler(const Request& req)
		// {
		// 	thread_local auto memc = std::unique_ptr<memcached_st>(memcached_create(nullptr));
		// 	memcached_server_add(memc.get(), _host.c_str(), _port);
		// 	size_t len = req.value_size;
		// 	uint32_t flags;
		//  std::this_thread::sleep_until(start_time + std::chrono::seconds(req.timestamp));
		// 	if (req.operation == Operation::op_get) {
		// 		char* val = memcached_get(memc.get(), req.key, req.key_size, &len, &flags, NULL);
		// 	}
		// 	else if (req.operation == Operation::op_set) {
		// 		memcached_set(memc.get(), req.key, req.key_size, _data_source->data(),
		// 					  req.value_size, 0, 0);
		// 	}
		// }

	private:
		inline static thread_local Statistics _stats;
		inline static Statistics _total_stats;

		Request_handler _request_handler;
		Request_buffer* _request_buffer;
		Data_source* _data_source;
		std::vector<std::jthread> _pool;
		int _pool_size;
		std::string _host;
		int _port;
	};
} // namespace KV_trace

#endif // __CLIENT_POOL_HPP__