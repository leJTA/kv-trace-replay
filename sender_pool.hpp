#ifndef __SENDER_POOL_HPP__
#define __SENDER_POOL_HPP__

#include <chrono>
#include <thread>
#include <vector>

#include "request.hpp"

class Sender_pool {
public:
	Sender_pool(int pool_size, Request_buffer& request_buffer)
		: _pool_size{pool_size}, _request_buffer{request_buffer}
	{}
	void start();
	void stop();

private:
	std::vector<std::jthread> _senders;
	int _pool_size;
	Request_buffer& _request_buffer;
	const std::chrono::steady_clock::time_point _replay_start = std::chrono::steady_clock::now();
};

#endif // __SENDER_POOL_HPP__