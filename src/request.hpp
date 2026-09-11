#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <charconv>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <queue>
#include <ranges>
#include <string_view>
#include <vector>

#include "bounded_queue.hpp"

namespace KV_trace {
	enum class Operation {
		OP_GET,
		OP_SET,
		OP_DELETE,
		OP_UNSUPPORTED
	};

	struct Request {
		uint32_t timestamp;
		const char* key;
		uint8_t key_size;
		uint32_t value_size;
		uint8_t client_id;
		Operation operation;
		uint32_t ttl;
	};
	using Request_buffer = Bounded_queue<Request>;

	static Operation _op_from_str(std::string_view op)
	{
		if (op == "get") {
			return Operation::OP_GET;
		}
		else if (op == "set") {
			return Operation::OP_SET;
		}
		else if (op == "deleted") {
			return Operation::OP_DELETE;
		}
		else {
			return Operation::OP_UNSUPPORTED;
		}
	}

	Request request_from_csv_line(std::string_view line)
	{
		std::vector<std::string> fields =
			line | std::views::split(',') | std::ranges::to<std::vector<std::string>>();

		// Twitter Twemcache format
		// - fields[0] timestamp in sec
		// - fields[1] anonymized key
		// - fields[2] key size in bytes
		// - fields[3] value size in bytes
		// - fields[4] client id
		// - fields[5] operation: one of
		// get/gets/set/add/replace/cas/append/prepend/delete/incr/decr
		// - fields[6] TTL (time-to-live)

		Request req;

		std::from_chars(fields[0].data(), fields[0].data() + fields[0].size(), req.timestamp);
		req.key = strdup(fields[1].c_str());
		std::from_chars(fields[2].data(), fields[2].data() + fields[2].size(), req.key_size);
		std::from_chars(fields[3].data(), fields[3].data() + fields[3].size(), req.value_size);
		std::from_chars(fields[4].data(), fields[4].data() + fields[4].size(), req.client_id);
		req.operation = _op_from_str(fields[5].c_str());
		std::from_chars(fields[6].data(), fields[6].data() + fields[6].size(), req.ttl);

		std::cout << req.timestamp << " " << req.key << ", " << fields[5] << " " << req.value_size
				  << "\n";

		return req;
	}
} // namespace KV_trace

#endif // __REQUEST_HPP__