#include "gl_object.h"

#include <vector>
#include <stdint.h>

struct VertexBufferLayout 
{
	size_t size   { 0 };
	size_t stride { 0 };
	size_t offset { 0 };
	int divisor 	{ 0 };
};

class VBO : public GLObject
{
public:
	enum class Usage
	{
		Static,
		Dynamic,
		Stream
	};

	enum class Type
	{
		Array,
		Indices
	};

	VBO(uint32_t attribute, Type type, Usage usage, size_t amount = 0, size_t size = 0, const void *data = nullptr, std::vector<VertexBufferLayout> layouts = {});
	~VBO() override;

	void bind() override;
	void unbind() override;

	void store(void* data, int amount) const;

	template <typename T>
	void store(const std::vector<T>& vec) const
	{
		VBO::store(vec.data(), vec.size());
	}

	void update(void* data, int amount, int pos = 0) const;

	template <typename T>
	void update(const std::vector<T>& vec, int pos = 0) const
	{
		VBO::update(vec.data(), vec.size(), pos);
	}

	void* map() const;
	bool unmap() const;

	uint32_t get_usage() const;
	uint32_t get_type() const;
	const std::vector<uint32_t> &get_used_attributes() const;
	uint32_t get_index_count() const;
	size_t get_size() const;

private:
	std::vector<uint32_t> attributes;

	uint32_t index_count;
	uint32_t type;
	uint32_t usage;
	size_t size;

	static uint32_t vbo_usage_to_gl_usage(VBO::Usage vbo_usage);
	static uint32_t vbo_type_to_gl_type(VBO::Type vbo_type);

	VBO(const VBO &) = delete;
	VBO &operator=(const VBO &) = delete;
};