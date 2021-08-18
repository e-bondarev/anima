#include "gl/vbo.h"

#include <GL/glew.h>

VBO::VBO(uint32_t attribute, Type type, Usage usage, size_t amount, size_t size, const void *data, std::vector<VertexBufferLayout> layouts)
{
    this->usage = vbo_usage_to_gl_usage(usage);
    this->type = vbo_type_to_gl_type(type);
    this->size = size;

    uint32_t startAttribute = attribute;
    attributes.resize(layouts.size());

    glGenBuffers(1, &handle);
    glBindBuffer(this->type, handle);

    if (attribute != -1)
    {
        glEnableVertexAttribArray(attribute);
        for (int i = 0; i < attributes.size(); i++)
        {
            int attrib = startAttribute + i;
            glVertexAttribPointer(attrib, layouts[i].size, GL_FLOAT, GL_FALSE, layouts[i].stride, reinterpret_cast<void *>(layouts[i].offset));
            glVertexAttribDivisor(attrib, layouts[i].divisor);

            attributes[i] = attrib;
        }
        glDisableVertexAttribArray(attribute);
    }

    glBufferData(this->type, size * amount, data, this->usage);
    glBindBuffer(this->type, 0);

    if (type == VBO::Type::Indices)
    {
        index_count = amount;
    }
}

VBO::~VBO()
{
    glBindBuffer(this->type, 0);
    glDeleteBuffers(1, &handle);

    // MW_DEBUG_LOG_OUT("[Call] VBO destructor");
}

void* VBO::map() const
{
	return glMapBuffer(type, GL_WRITE_ONLY);
}

void VBO::bind()
{
    glBindBuffer(type, handle);
}

void VBO::unbind()
{
    glBindBuffer(type, 0);
}

bool VBO::unmap() const
{
    return glUnmapBuffer(type);
}

uint32_t VBO::get_usage() const
{
    return usage;
}

uint32_t VBO::get_type() const
{
    return type;
}

const std::vector<uint32_t>& VBO::get_used_attributes() const
{
    return attributes;
}

void VBO::store(void* data, int amount) const
{
	glBufferData(type, amount * size, data, usage);
}

void VBO::update(void* data, int amount, int pos) const
{
	glBufferSubData(type, size * pos, size * amount, data);
}

uint32_t VBO::get_index_count() const
{
    return index_count;
}

size_t VBO::get_size() const
{
    return size;
}

uint32_t VBO::vbo_usage_to_gl_usage(VBO::Usage vbo_usage)
{
    switch (vbo_usage)
    {
    case VBO::Usage::Static:
        return GL_STATIC_DRAW;
    case VBO::Usage::Dynamic:
        return GL_DYNAMIC_DRAW;
    case VBO::Usage::Stream:
        return GL_STREAM_DRAW;
    }

    return GL_STATIC_DRAW;
}

uint32_t VBO::vbo_type_to_gl_type(VBO::Type vbo_type)
{
    switch (vbo_type)
    {
    case VBO::Type::Array:
        return GL_ARRAY_BUFFER;
    case VBO::Type::Indices:
        return GL_ELEMENT_ARRAY_BUFFER;
    }

    return GL_ARRAY_BUFFER;
}