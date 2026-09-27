#ifndef __TRACE_PRODUCER_HPP__
#define __TRACE_PRODUCER_HPP__

#include <string>

#include "request.hpp"
#include "system_error"

namespace KV_trace {
	class Trace_producer {
	public:
		explicit Trace_producer(const std::string& trace_file, int64_t limit)
			: _trace_file{trace_file}, _request_buffer{nullptr}, _max_count{limit}
		{}

		void attach_request_buffer(Request_buffer& request_buffer)
		{
			_request_buffer = &request_buffer;
		}

		std::error_code start()
		{
			std::ifstream trace{_trace_file};
			if (!trace.is_open()) {
				return Trace_error::trace_file_open_failed;
			}

			std::string line;
			int count = 0;
			while (std::getline(trace, line) && (_max_count < 0 || count++ < _max_count)) {
				_request_buffer->push(request_from_csv_line(line));
			}
			trace.close();

			return {};
		}

	private:
		std::string _trace_file;
		Request_buffer* _request_buffer;
		const int64_t _max_count;
	};
} // namespace KV_trace

#endif // __TRACE_PRODUCER_HPP__