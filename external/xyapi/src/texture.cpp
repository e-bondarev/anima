#include "gl/texture.h"

#include <GL/glew.h>

Texture::Texture(uint32_t width, uint32_t height, Pixels_t data, GLint internalFormat, GLuint format, GLuint type, Parameters_t params) : width{width}, height{height}, internalFormat{internalFormat}, format{format}, type{type}
{
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);

	for (int i = 0; i < params.size(); i++)
	{
		params[i]();
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &handle);
}

uint32_t Texture::get_width() const
{
	return width;
}

uint32_t Texture::get_height() const
{
	return height;
}

void Texture::resize(uint32_t width, uint32_t height)
{
	this->width = width; this->height = height;
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
}

void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, handle);
}

void Texture::bind(GLuint unit)
{
	glActiveTexture(unit);
	bind();
}

void Texture::unbind(GLuint unit)
{
	glActiveTexture(unit);
	unbind();
}

void Texture::unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void LinearInterpolation()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void ConstantInterpolation()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Texture::Parameter_t Texture::set_interpolation(Interpolation interpolation)
{
	return interpolation == Interpolation::Linear ? LinearInterpolation : ConstantInterpolation;
}