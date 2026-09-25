#ifndef __STATISTICS_HPP__
#define __STATISTICS_HPP__

#include <atomic>
#include <mutex>
#include <vector>

#include <hdr/hdr_histogram.h>

namespace KV_trace {
	class Statistics {
	public:
		Statistics(): _time_histogram{nullptr, &hdr_close}
		{
			hdr_histogram* hist;
			// min time = 1ns, max time = 60s
			hdr_init(1, INT64_C(60'000'000), 3, &hist);
			_time_histogram.reset(hist);
		}

		void record_time(int64_t value) { hdr_record_value(_time_histogram.get(), value); }
		void add(const Statistics& from)
		{
			std::lock_guard<std::mutex> lock{_mut};
			hdr_add(_time_histogram.get(), from._time_histogram.get());
		}
		void record_total_time(int64_t total_time) { _total_time = total_time; }

		int64_t percentile(double percentile) const
		{
			return hdr_value_at_percentile(_time_histogram.get(), percentile);
		};
		int64_t mean() const { return hdr_mean(_time_histogram.get()); }
		int64_t min() const { return hdr_min(_time_histogram.get()); }
		int64_t max() const { return hdr_max(_time_histogram.get()); }
		int64_t total_count() const { return _time_histogram->total_count; }
		int64_t total_time() const { return _total_time; }
		std::vector<std::pair<double, double>> cdf() const
		{
			std::vector<std::pair<double, double>> cdf;
			for (int i = 1; i <= 1000; ++i) {
				double p = i / 10.0;
				cdf.emplace_back(p, hdr_value_at_percentile(_time_histogram.get(), p));
			}
			return cdf;
		}

	private:
		std::unique_ptr<hdr_histogram, decltype(&hdr_close)> _time_histogram;
		int64_t _total_time;
		std::mutex _mut;
	};
} // namespace KV_trace

#endif // __STATISTICS_HPP__