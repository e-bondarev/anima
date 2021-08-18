#include <vector> 
#include <string> 
inline static const std::string default_vert = R""""( 
#version 440 core

layout (location = 0) in vec2 in_Position;

uniform vec2 u_Float;

void main()
{
	gl_Position = vec4(in_Position + u_Float, 0.0, 1.0);
}

)"""";