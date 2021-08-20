#pragma once

#include <string>

class Image
{
public:
	Image(const std::string& path);
	~Image();

	int width, height;
	int num_of_channels;
	unsigned char* data;

private:
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;
};