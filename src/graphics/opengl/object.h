#pragma once

#include <GL/glew.h>

class GPUObject
{
public:
	GPUObject() {}
	virtual ~GPUObject() {}

	virtual void bind() = 0;
	virtual void unbind() = 0;

	GLuint get_handle() const;

protected:
	GLuint handle;
};