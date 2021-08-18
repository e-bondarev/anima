#include "files/files.h"

#include "common.h"

const std::vector<std::string> SHADER_FORMATS = { "vert", "frag" };

bool is_shader(const std::string& format)
{
	for (int i = 0; i < SHADER_FORMATS.size(); i++)
		if (SHADER_FORMATS[i] == format)
			return true;

	return false;
}

void file_callback(const std::string& entry)
{
	const std::string& type = files::get_type(entry);

	if (is_shader(type))
	{
		const std::string content = files::read(entry);

		const std::string header_path = entry + ".h";
		files::write(header_path, content);
	}
}

void directory_callback(const std::string& entry)
{
	files::recursive_loop(entry, directory_callback, file_callback);
}

void compile_shaders()
{
	spdlog::info("Compiling shaders..");

	const std::string path = get_arguments()[0];
	const std::string src = path.substr(0, path.find("build")) + "src";
	
	files::recursive_loop(src, directory_callback, file_callback);
}

#ifdef COMPILE_SHADERS
int main(int argc, char* argv[])
{
	arguments(argc, argv);

	compile_shaders();
}
#endif