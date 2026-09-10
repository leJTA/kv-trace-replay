#ifndef __CLIENT_POOL_HPP__
#define __CLIENT_POOL_HPP__

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include <vector>

#include "request.hpp"

class Client_pool {
public:
	std::chrono::steady_clock::time_point start_time;

	explicit Client_pool(int pool_size)
		: _request_buffer{nullptr}, _pool_size{pool_size}, _done{false}
	{}

	void set_request_handler(std::function<void(const Request&)> handler)
	{
		_request_handler = handler;
	}
	void attach_request_buffer(Request_buffer& request_buffer)
	{
		_request_buffer = &request_buffer;
	}

	void start()
	{
		start_time = std::chrono::steady_clock::now();
		for (int i = 0; i < _pool_size; ++i) {
			_pool.emplace_back([this]() {
				while (true) {
					Request request = this->_request_buffer->pop();
					this->_request_handler(request);
				}
			});
		}
	}
	void stop() { _done = true; }

private:
	std::function<void(const Request&)> _request_handler;
	Request_buffer* _request_buffer;
	std::vector<std::jthread> _pool;
	int _pool_size;
	std::atomic_bool _done;
};

#endif // __CLIENT_POOL_HPP__