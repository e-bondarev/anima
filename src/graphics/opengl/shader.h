// #pragma once

// #include "object.h"

// #include <string>

// class Shader : public GPUObject
// {
// public:
// 	Shader(const std::string& vs_code, const std::string& fs_code);
// 	~Shader();

// 	void bind() override;
// 	void unbind() override;

// private:
// 	GLuint create_shader(const std::string code, GLuint shader_type);
// 	void link() const;

// 	GLuint vs_handle;
// 	GLuint fs_handle;
// };