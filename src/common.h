#pragma once

#include <vector>
#include <string>

void arguments(int argc, char* argv[]);
const std::vector<std::string>& get_arguments();

#include <spdlog/spdlog.h>

#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>