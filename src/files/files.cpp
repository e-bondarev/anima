#include "files.h"

#include <fstream>

#include <spdlog/spdlog.h>

namespace files
{
	void delete_entry(const std::string& entry)
	{
		std::remove(entry.c_str());
	}

	void recursive_loop(const std::string& home, const callback_t& directory_callback, const callback_t& file_callback)
	{
		for (const auto& entry : std::filesystem::directory_iterator(home))
		{
			if (entry.is_directory())
				directory_callback(entry.path().string());
			else
				file_callback(entry.path().string());
		}
	}

	std::string get_type(const std::string& path)
	{
		const int point_index = path.find_last_of(".");

		return path.substr(point_index + 1, path.size());
	}

	std::string read(const std::string& path)
	{
		std::string fff;

		std::ifstream file(path);

		if (!file.is_open())
		{
			spdlog::error("Failed to open file: {0}", path);
		}

		for( std::string line; std::getline( file, line ); )
		{
			fff.append(line + "\n");
		}

		return fff;
	}

	static std::string path_to_var_name(const std::string& path)
	{
		const int last_slash_index_0 = path.find_last_of("/");
		const int last_slash_index_1 = path.find_last_of("\\");
		const int last_slash_index = std::max(last_slash_index_0, last_slash_index_1);
		const std::string file_name = path.substr(last_slash_index + 1, path.size());

		const static std::string cut = ".h";
		const static std::vector<char> symbols_to_change = { '-', '.' };

		std::string var_name = file_name.substr(0, file_name.size() - cut.size());	

		for (int i = 0; i < var_name.size(); i++)
		{
			for (int j = 0; j < symbols_to_change.size(); j++)
			{
				if (var_name[i] == symbols_to_change[j])
				{
					var_name[i] = '_';
				}
			}
		}

		return var_name;
	}

	void write(const std::string& path, const std::string& data)
	{
		std::ofstream file;
		file.open(path);

		const std::string var_name = path_to_var_name(path);

		file << "#include <vector> \n";
		file << "#include <string> \n";
		file << "inline static const std::string ";
		file << var_name;
		file << " = R\"\"\"\"( \n";

		file << data;

		file << "\n)\"\"\"\";";
		file.close();
	}
}
