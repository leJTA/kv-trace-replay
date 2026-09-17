#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include "bounded_queue.hpp"

namespace KV_trace {
	enum class Operation {
		op_get,
		op_set,
		op_delete,
		op_unsupported
	};

	struct Request {
		uint32_t timestamp;
		Operation operation;
		char key[128];
		uint32_t value_size;
		uint32_t ttl;
		uint8_t key_size;
		uint8_t client_id;
	};

	using Request_buffer = Bounded_queue<Request>;

	Request request_from_csv_line(std::string_view line);
} // namespace KV_trace

#endif // __REQUEST_HPP__