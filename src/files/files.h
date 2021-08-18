#pragma once

#include <filesystem>
#include <functional>
#include <stdint.h>
#include <string>
#include <vector>

namespace files
{
	void delete_entry(const std::string& entry);

	using callback_t = std::function<void(const std::string&)>;

	void recursive_loop(const std::string& home, const callback_t& directory_callback, const callback_t& file_callback);

	std::string get_type(const std::string& path);

	std::string read(const std::string& path);

	void write(const std::string& path, const std::string& data);
}
