#include "gl/shader.h"
#include "common.h"

#include <GL/glew.h>

Shader::Shader(const std::string& vs_code, const std::string& fs_code, const std::vector<std::string>& uniform_variables)
{
	handle = glCreateProgram();

	vs_handle = create_shader(vs_code, GL_VERTEX_SHADER);
	fs_handle = create_shader(fs_code, GL_FRAGMENT_SHADER);
	link();
	create_uniforms(uniform_variables);
}

Shader::~Shader()
{
	unbind();
	glDetachShader(handle, vs_handle);
	glDetachShader(handle, fs_handle);
	glDeleteShader(vs_handle);
	glDeleteShader(fs_handle);
	glDeleteProgram(handle);
}

void Shader::bind()
{
	glUseProgram(handle);
}

void Shader::unbind()
{
	glUseProgram(0);
}

GLuint Shader::create_shader(const std::string code, GLuint shader_type)
{
	const unsigned int shader_id = glCreateShader(shader_type);

	const char* c_str = code.c_str();

	glShaderSource(shader_id, 1, &c_str, NULL);
	glCompileShader(shader_id);

	int status;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);

	if (!status)
	{
		int length;
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
		std::string log;
		log.resize(length);
		glGetShaderInfoLog(shader_id, length, &length, &log[0]);

		std::string errorFunctionName = "--------[ " __FUNCTION__ " ]--------";
		std::string separator; for (size_t i = 0; i < errorFunctionName.size(); i++) separator += "-";

		info_callback(log);

		return -1;
	}

	glAttachShader(handle, shader_id);

	return shader_id;
}

void Shader::link() const
{
	glLinkProgram(handle);

	if (vs_handle != 0)
	{
		glDetachShader(handle, vs_handle);
	}

	if (fs_handle != 0)
	{
		glDetachShader(handle, fs_handle);
	}

	glValidateProgram(handle);
}

void Shader::create_uniforms(const std::vector<std::string>& uniform_variables)
{
	for (int i = 0; i < uniform_variables.size(); i++)
	{
		int32_t location = glGetUniformLocation(handle, uniform_variables[i].c_str());
		if (location != -1)
		{
			uniforms[uniform_variables[i]] = location;
		}
	}
}

void Shader::set_uniform_vec2(const std::string& name, float* data)
{
	glUniform2f(uniforms[name], data[0], data[1]);
}

void Shader::set_uniform_mat4(const std::string& name, float* data, uint32_t count)
{
	glUniformMatrix4fv(uniforms[name], count, GL_FALSE, data);
}