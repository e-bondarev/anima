#pragma once

#include <functional>

using info_callback_t = std::function<void(const std::string& msg)>;
using error_callback_t = info_callback_t;
using warning_callback_t = info_callback_t;

extern info_callback_t info_callback;
extern error_callback_t error_callback;
extern warning_callback_t warning_callback;