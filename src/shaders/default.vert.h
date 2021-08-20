#include <vector> 
#include <string> 
inline static const std::string default_vert = R""""( 
#version 440 core

layout (location = 0) in  vec3 in_position;
layout (location = 1) in  vec2 in_uv;
layout (location = 2) in  vec3 in_normal;
layout (location = 3) in ivec4 in_bone_indices;
layout (location = 4) in  vec4 in_weights;

out struct {
	vec2 uv;
} vs_out;

const int MAX_BONES = 100;

uniform mat4 u_model;
uniform mat4 u_proj;
uniform mat4 u_bones[MAX_BONES];

void main()
{	
	mat4 bone_transform = u_bones[in_bone_indices[0]] * in_weights[0];
		bone_transform += u_bones[in_bone_indices[1]] * in_weights[1];
		bone_transform += u_bones[in_bone_indices[2]] * in_weights[2];
		bone_transform += u_bones[in_bone_indices[3]] * in_weights[3];

	gl_Position = 
		u_proj * 
		u_model * 
		bone_transform * 
		vec4(in_position, 1.0);
	vs_out.uv = in_uv;
}

)"""";