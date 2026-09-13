#ifndef __TRACE_PLAYER_HPP__
#define __TRACE_PLAYER_HPP__

#include <memory>
#include <print>
#include <string_view>

#include <libmemcached/memcached.h>

#include "client_pool.hpp"
#include "config.hpp"
#include "data_source.hpp"
#include "request.hpp"
#include "trace_error.hpp"
#include "trace_producer.hpp"

static constexpr size_t _max_data_size = 256 * 1024; // 256 KB

namespace KV_trace {
	class Trace_player {
	public:
		explicit Trace_player(const Config& config)
			: _data_file{config.data_file}, _data_source{_max_data_size},
			  _request_buffer{config.buffer_size}, _trace_producer{config.trace_file},
			  _client_pool{config.nthreads, config.host, config.port}
		{}

		std::error_code run()
		{
			std::error_code ec;
			ec = _data_source.load(_data_file);
			if (ec) {
				return ec;
			}

			_trace_producer.attach_request_buffer(_request_buffer);
			_client_pool.set_data_source(&_data_source);
			_client_pool.attach_request_buffer(&_request_buffer);

			_client_pool.set_request_handler(
				[this](const Request& r) { _client_pool.console_request_handler(r); });
			_client_pool.start();

			ec = _trace_producer.start();
			if (ec) {
				_request_buffer.close();
				return ec;
			}

			// close the queue to notify clients so that they do not get stuck waiting for new
			// requests
			_request_buffer.close();
			return {};
		}

	private:
		std::string _data_file;
		Data_source _data_source;
		Request_buffer _request_buffer;
		Trace_producer _trace_producer;
		Client_pool _client_pool;
	};
} // namespace KV_trace

#endif // __TRACE_PLAYER_HPP__