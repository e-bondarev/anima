#pragma once

#include <functional>
#include <stdint.h>
#include <vector>

#include "gl_object.h"

enum class Interpolation
{
	Linear,
	Constant
};

class Texture : public GLObject 
{
public:
	using Pixels_t = const unsigned char*;

	using Parameter_t = std::function<void()>;
	using Parameters_t = std::vector<Parameter_t>;

	Texture(uint32_t width, uint32_t height, Pixels_t data, int32_t internalFormat, uint32_t format, uint32_t type, Parameters_t params = {});
	~Texture() override;

	static Parameter_t set_interpolation(Interpolation interpolation);

	void resize(uint32_t width, uint32_t height);

	void bind() override;
	void bind(uint32_t unit);
	void unbind() override;
	void unbind(uint32_t unit);

	uint32_t get_width() const;
	uint32_t get_height() const;

	
protected:
	uint32_t width, height;

	int32_t internalFormat;
	uint32_t format;
	uint32_t type;

private:
	Texture(const Texture&) = delete;
	Texture operator=(const Texture&) = delete;
};