#include "gl/vao.h"

#include <GL/glew.h>

VAO::VAO()
{
	glGenVertexArrays(1, &handle);
	// MW_DEBUG_LOG_OUT("[Call] Vao constructor");
}

VAO::~VAO()
{
	unbind();
	glDeleteVertexArrays(1, &handle);

	// MW_DEBUG_LOG_OUT("[Call] Vao destructor");
}

uint32_t VAO::get_vertex_count() const
{
	return vertex_count;
}

void VAO::bind()
{
	glBindVertexArray(handle);

	for (const auto &attribute : attributes)
	{
		glEnableVertexAttribArray(attribute);
	}
}

void VAO::unbind()
{
	for (int i = attributes.size() - 1; i >= 0; --i)
	{
		glDisableVertexAttribArray(attributes[i]);
	}

	glBindVertexArray(0);
}

uint32_t VAO::get_last_attribute() const
{
	return static_cast<uint32_t>(attributes.size());
}

const std::shared_ptr<VBO>& VAO::get_index_buffer() const
{
	return index_buffer;
}