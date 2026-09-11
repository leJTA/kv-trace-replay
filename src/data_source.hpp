#ifndef __DATA_SOURCE_HPP__
#define __DATA_SOURCE_HPP__

#include <fstream>
#include <memory>
#include <string>

#include "trace_error.hpp"

namespace KV_trace {
	class Data_source {
	public:
		explicit Data_source(size_t size): _data(size) {}

		std::error_code load(const std::string& data_file)
		{
			std::ifstream input{data_file, std::ios::binary};
			if (!input) {
				return Trace_error::data_file_open_failed;
			}

			input.read(_data.data(), static_cast<std::streamsize>(_data.size()));
			if (input.gcount() != static_cast<std::streamsize>(_data.size())) {
				return Trace_error::data_file_too_small;
			}

			return {};
		}

		const char* data() const { return _data.data(); }
		size_t size() const { return _data.size(); }

	private:
		std::vector<char> _data;
	};
} // namespace KV_trace

#endif // __DATA_SOURCE_HPP__