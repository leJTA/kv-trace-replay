#ifndef __TRACE_PLAYER_HPP__
#define __TRACE_PLAYER_HPP__

#include <fstream>
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
			  _client_pool{config.nthreads, config.host, config.port},
			  _output_csv{config.output_csv}
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
			_client_pool.wait();

			if (!_output_csv.empty()) {
				std::ofstream csv{_output_csv, std::ios::app};
				if (!csv.is_open()) {
					return Trace_error::csv_file_open_failed;
				}
				std::println(csv, "requests,hits,misses,min,max,mean,median,p90,p95,p99,p99.9");
				std::println(csv, "{},{},{},{},{},{},{},{},{},{},{}",
							 _client_pool.statistics()->total_count(),
							 _client_pool.statistics()->hits(), _client_pool.statistics()->misses(),
							 _client_pool.statistics()->min(), _client_pool.statistics()->max(),
							 _client_pool.statistics()->mean(),
							 _client_pool.statistics()->percentile(50),
							 _client_pool.statistics()->percentile(90),
							 _client_pool.statistics()->percentile(95),
							 _client_pool.statistics()->percentile(99),
							 _client_pool.statistics()->percentile(99.9));
			}

			return {};
		}

		Statistics* statistics() const { return _client_pool.statistics(); }

	private:
		std::string _data_file;
		Data_source _data_source;
		Request_buffer _request_buffer;
		Trace_producer _trace_producer;
		Client_pool _client_pool;
		std::string _output_csv;
	};
} // namespace KV_trace

#endif // __TRACE_PLAYER_HPP__