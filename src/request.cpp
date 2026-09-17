#include "request.hpp"

#include <charconv>
#include <cstring>
#include <ranges>
#include <string_view>
#include <vector>

namespace KV_trace {
	static Operation _op_from_str(std::string_view op)
	{
		if (op == "get") {
			return Operation::op_get;
		}
		else if (op == "set") {
			return Operation::op_set;
		}
		else if (op == "deleted") {
			return Operation::op_delete;
		}
		else {
			return Operation::op_unsupported;
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
		strcpy(req.key, fields[1].c_str());
		std::from_chars(fields[2].data(), fields[2].data() + fields[2].size(), req.key_size);
		std::from_chars(fields[3].data(), fields[3].data() + fields[3].size(), req.value_size);
		std::from_chars(fields[4].data(), fields[4].data() + fields[4].size(), req.client_id);
		req.operation = _op_from_str(fields[5].c_str());
		std::from_chars(fields[6].data(), fields[6].data() + fields[6].size(), req.ttl);

		return req;
	}
} // namespace KV_trace