#pragma once

#include <stdint.h>

class GLObject
{
public:
	GLObject() = default;
	virtual ~GLObject() = default;

	virtual void bind() = 0;
	virtual void unbind() = 0;

	inline uint32_t get_handle() const
	{
		return handle;
	}

protected:
	uint32_t handle;
};