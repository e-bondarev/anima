#include "vao.h"

VAO::~VAO()
{
	unbind();
	glDeleteBuffers(m_Buffers.size(), m_Buffers.data());
	glDeleteVertexArrays(1, &handle);
}

void VAO::bind()
{
	glBindVertexArray(handle);

	for (const auto& attribute : m_Attributes)
	{
		glEnableVertexAttribArray(attribute);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexVboHandle);
}

void VAO::unbind()
{
	glBindVertexArray(0);

	for (int i = m_Attributes.size() - 1; i >= 0; --i)
	{
		glDisableVertexAttribArray(m_Attributes[i]);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLuint VAO::GetVertexCount() const
{
	return m_VertexCount;
}