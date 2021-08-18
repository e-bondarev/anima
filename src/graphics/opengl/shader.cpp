#include "shader.h"

#include <iostream>

Shader::Shader(const std::string& vs_code, const std::string& fs_code)
{
	handle = glCreateProgram();

	vs_handle = create_shader(vs_code, GL_VERTEX_SHADER);
	fs_handle = create_shader(fs_code, GL_FRAGMENT_SHADER);
	link();
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

		std::cout << log << std::endl;

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