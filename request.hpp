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

enum class Operation {
	GET,
	SET,
	DELETE,
};

struct Request {
	uint32_t timestamp;
	const char* key;
	uint8_t key_size;
	uint32_t value_size;
	uint8_t client_id;
	Operation operation;
	uint32_t ttl;

	static Request parse_line(std::string_view line)
	{
		std::vector<std::string> fields =
			line | std::views::split(',') | std::ranges::to<std::vector<std::string>>();

		// fields[0] timestamp in sec
		// fields[1] anonymized key
		// fields[2] key size in bytes
		// fields[3] value size in bytes
		// fields[4] client id
		// fields[5] operation: one of get/gets/set/add/replace/cas/append/prepend/delete/incr/decr
		// fields[6] TTL (time-to-live)

		Request req;

		std::from_chars(fields[0].data(), fields[0].data() + fields[0].size(), req.timestamp);
		req.key = strdup(fields[1].c_str());
		std::from_chars(fields[2].data(), fields[2].data() + fields[2].size(), req.key_size);
		std::from_chars(fields[3].data(), fields[3].data() + fields[3].size(), req.value_size);
		std::from_chars(fields[4].data(), fields[4].data() + fields[4].size(), req.client_id);
		req.operation = (fields[5] == "get" ? Operation::GET : Operation::SET);
		std::from_chars(fields[6].data(), fields[6].data() + fields[6].size(), req.ttl);

		return req;
	}
};

using Request_buffer = Bounded_queue<Request>;

#endif // __REQUEST_HPP__