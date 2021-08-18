#pragma once

#include <vector>
#include <string>

void arguments(int argc, char* argv[]);
const std::vector<std::string>& get_arguments();

#include <spdlog/spdlog.h>