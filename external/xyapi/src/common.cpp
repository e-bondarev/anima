#include "common.h"

#include <iostream>

static void default_callback(const std::string& msg)
{
	std::cout << msg << std::endl;
}

info_callback_t info_callback = default_callback;
error_callback_t error_callback = default_callback;
warning_callback_t warning_callback = default_callback;