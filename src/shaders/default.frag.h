#include <vector> 
#include <string> 
inline static const std::string default_frag = R""""( 
#version 440 core

out vec4 out_Color;

void main()
{
	out_Color = vec4(0.0, 1.0, 1.0, 1.0);
}

)"""";