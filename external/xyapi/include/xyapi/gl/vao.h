#pragma once

#include "gl_object.h"
#include "vbo.h"

#include <memory>

class VAO : public GLObject
{
public:
	VAO();
	~VAO() override;

	void bind() override;
	void unbind() override;

	uint32_t get_vertex_count() const;

	template <typename... Args>
	inline std::shared_ptr<VBO> add_vbo(Args... args)
	{
		std::shared_ptr<VBO> vbo = std::make_shared<VBO>(get_last_attribute(), std::forward<Args>(args)...);

		auto used_attributes = vbo->get_used_attributes();
		for (int i = 0; i < used_attributes.size(); i++)
		{
			attributes.push_back(used_attributes[i]);
		}

		vbos.push_back(vbo);

		if (vbo->get_type() == /*GL_ELEMENT_ARRAY_BUFFER*/ 0x8893)
		{
			index_buffer = vbo;
			vertex_count = vbo->get_index_count();
		}

		return vbo;
	}

	uint32_t get_last_attribute() const;
	const std::shared_ptr<VBO>& get_index_buffer() const;

private:
	std::vector<std::shared_ptr<VBO>> vbos;
	std::shared_ptr<VBO> index_buffer;

	uint32_t vertex_count{0};

	std::vector<uint32_t> attributes;
	std::vector<uint32_t> buffers;

	VAO(const VAO &) = delete;
	VAO &operator=(const VAO &) = delete;
};