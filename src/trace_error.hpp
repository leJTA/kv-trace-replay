#ifndef __TRACE_ERROR_HPP__
#define __TRACE_ERROR_HPP__

#include <cassert>
#include <expected>
#include <string>
#include <system_error>
#include <utility>

namespace KV_trace {
	enum class Trace_error {
		trace_file_open_failed = 1,
		data_file_open_failed,
		data_file_too_small,
	};

	struct Trace_error_category : std::error_category {
		const char* name() const noexcept override { return "kv_trace"; }
		std::string message(int ev) const override
		{
			switch (static_cast<Trace_error>(ev)) {
			case Trace_error::trace_file_open_failed:
				return "unable to open trace file";
			case Trace_error::data_file_open_failed:
				return "unable to open data file";
			case Trace_error::data_file_too_small:
				return "data source file is too small";
			default:
				return "unknown error";
			}
		}
	};

	// Mapping from error code enum to category
	inline std::error_code make_error_code(Trace_error e)
	{
		static auto category = Trace_error_category{};
		return std::error_code(std::to_underlying(e), category);
	}

} // namespace KV_trace

// Register the enum as an error code enum
template<> struct std::is_error_code_enum<KV_trace::Trace_error> : public std::true_type {};

#endif // __TRACE_ERROR_HPP__