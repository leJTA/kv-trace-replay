#include "sender_pool.hpp"

#include <thread>

void Sender_pool::start()
{
	for (int i = 0; i < _pool_size; ++i) {
		_senders.push_back([this]() { Request request = this->_request_buffer.pop(); });
	}
}