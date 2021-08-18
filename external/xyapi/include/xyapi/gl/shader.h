#pragma once

#include <string>
#include <vector>
#include <map>

#include "gl_object.h"

class Shader : public GLObject
{
public:
	Shader(const std::string& vs_code, const std::string& fs_code, const std::vector<std::string>& uniform_variables = {});
	~Shader() override;

	void bind() override;
	void unbind() override;

	void set_uniform_vec2(const std::string& name, float* data);

private:
	uint32_t create_shader(const std::string code, uint32_t shader_type);
	void link() const;
	void create_uniforms(const std::vector<std::string>& uniform_variables);

	uint32_t vs_handle;
	uint32_t fs_handle;

	std::map<std::string, int32_t> uniforms;
};