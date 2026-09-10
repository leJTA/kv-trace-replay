#ifndef __DATA_PRODUCER_HPP__
#define __DATA_PRODUCER_HPP__

#include <fstream>
#include <memory>
#include <string>

class Data_source {
public:
	explicit Data_source(size_t size): _data(size) {}

	bool load(const std::string& data_file)
	{
		std::ifstream input{data_file, std::ios::binary};
		if (!input) {
			std::cout << "[ERROR] Unable to open file: " << data_file << "\n";
			return false;
		}

		input.read(_data.data(), static_cast<std::streamsize>(_data.size()));
		if (input.gcount() != static_cast<std::streamsize>(_data.size())) {
			std::cout << "[ERROR] Data file is too small\n";
			return false;
		}

		return true;
	}

	const char* data() const { return _data.data(); }
	size_t size() const { return _data.size(); }

private:
	std::vector<char> _data;
};

#endif // __DATA_PRODUCER_HPP__