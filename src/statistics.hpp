#ifndef __STATISTICS_HPP__
#define __STATISTICS_HPP__

#include <atomic>
#include <mutex>
#include <vector>

#include <hdr/hdr_histogram.h>

namespace KV_trace {
	class Statistics {
	public:
		Statistics()
		{
			// min time = 1ns, max time = 60s
			hdr_init(1, INT64_C(60000000), 3, &_time_histogram);
		}
		~Statistics() { hdr_close(_time_histogram); }

		void record_time(int64_t value) { hdr_record_value(_time_histogram, value); }
		void add(const Statistics& from)
		{
			std::lock_guard<std::mutex> lock{_mut};
			hdr_add(_time_histogram, from._time_histogram);
		}

		int64_t percentile(double percentile) const
		{
			return hdr_value_at_percentile(_time_histogram, percentile);
		};
		int64_t mean() const { return hdr_mean(_time_histogram); }
		int64_t min() const { return hdr_min(_time_histogram); }
		int64_t max() const { return hdr_max(_time_histogram); }
		int64_t total_count() const { return _time_histogram->total_count; }

	private:
		struct hdr_histogram* _time_histogram;
		std::mutex _mut;
	};
} // namespace KV_trace

#endif // __STATISTICS_HPP__