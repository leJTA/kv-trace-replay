#ifndef __SENDER_POOL_HPP__
#define __SENDER_POOL_HPP__

#include <chrono>
#include <functional>
#include <print>
#include <thread>
#include <vector>

#include <httplib.h>

#include "data_source.hpp"
#include "request.hpp"
#include "statistics.hpp"

namespace KV_trace {
	using Request_sender = std::function<void(const Request&)>;
	enum class Protocol {
		tcp,
		http,
		console
	};

	class Sender_pool {
	public:
		Sender_pool(int pool_size, const std::string& host, int port, Protocol protocol)
			: _request_buffer{nullptr}, _pool_size{pool_size}, _host{host}, _port{port},
			  _protocol{protocol}
		{
			switch (_protocol) {
			case Protocol::tcp:
				_request_sender = [this](const Request& req) { this->send_tcp_request(req); };
				break;
			case Protocol::http:
				_request_sender = [this](const Request& req) { this->send_http_request(req); };
				break;
			case Protocol::console:
				_request_sender = [this](const Request& req) { this->send_console_request(req); };
				break;
			default:
				break;
			}
		}

		void set_data_source(Data_source* data_source) { _data_source = data_source; }
		void attach_request_buffer(Request_buffer* request_buffer)
		{
			_request_buffer = request_buffer;
		}

		void start()
		{
			_start_time = std::chrono::steady_clock::now();
			for (int i = 0; i < _pool_size; ++i) {
				_pool.emplace_back([this]() {
					while (auto request = this->_request_buffer->pop()) [[likely]] {
						this->_request_sender(*request);
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
		void send_console_request(const Request& req)
		{
			// wait until the time to send the request arrives
			std::this_thread::sleep_until(_start_time + std::chrono::seconds(req.timestamp));

			auto begin = std::chrono::steady_clock::now();
			// console print request
			if (req.operation == Operation::op_get) {
				std::println("[{}] GET {}", req.timestamp, req.key);
			}
			else if (req.operation == Operation::op_set) {
				std::println("[{}] SET {} [size = {}, ttl = {}]", req.timestamp, req.key,
							 req.value_size, req.ttl);
			}
			else if (req.operation == Operation::op_delete) {
				std::println("[{}] DELETE {}", req.timestamp, req.key);
			}
			else [[unlikely]] {
				return;
			}
			auto end = std::chrono::steady_clock::now();
			_stats.record_time(
				std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count());
		}

		void send_http_request(const Request& req)
		{
			thread_local httplib::Client http_client{_host, _port};
			// wait until the time to send the request arrives
			std::this_thread::sleep_until(_start_time + std::chrono::seconds(req.timestamp));

			auto begin = std::chrono::steady_clock::now();
			// send http request
			if (req.operation == Operation::op_get) {
				http_client.Get(std::string("/").append(req.key));
			}
			else if (req.operation == Operation::op_set) {
				http_client.Post(std::string("/").append(req.key), _data_source->data(),
								 req.value_size, "application/octet-stream");
			}
			else if (req.operation == Operation::op_delete) {
				http_client.Delete(std::string("/").append(req.key));
			}
			else [[unlikely]] {
				return;
			}
			auto end = std::chrono::steady_clock::now();
			_stats.record_time(
				std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count());
		}

		void send_tcp_request(const Request& req)
		{
			// TODO : tcp client
			std::this_thread::sleep_until(_start_time + std::chrono::seconds(req.timestamp));

			auto begin = std::chrono::steady_clock::now();
			// TODO : send request, read response
			auto end = std::chrono::steady_clock::now();
			_stats.record_time(
				std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count());
		}

	private:
		inline static thread_local Statistics _stats;
		inline static Statistics _total_stats;

		Request_sender _request_sender;
		Request_buffer* _request_buffer;
		Data_source* _data_source;
		std::chrono::steady_clock::time_point _start_time;
		std::vector<std::jthread> _pool;
		int _pool_size;
		std::string _host;
		int _port;
		Protocol _protocol;
	};
} // namespace KV_trace

#endif // __SENDER_POOL_HPP__