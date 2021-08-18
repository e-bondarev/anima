#include "common.h"

std::vector<std::string> args;

void arguments(int argc, char* argv[])
{
	args.resize(argc);

	for (int i = 0; i < argc; i++)
	{
		args[i] = argv[i];
	}
}

const std::vector<std::string>& get_arguments()
{
	return args;
}