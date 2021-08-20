#include "image.h"

#include "stb_image.h"

Image::Image(const std::string& path)
{
	data = stbi_load(path.c_str(), &width, &height, &num_of_channels, STBI_rgb_alpha);
}

Image::~Image()
{
	stbi_image_free(data);
}