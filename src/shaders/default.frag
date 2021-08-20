#version 440 core

in struct {
	vec2 uv;
} vs_out;

out vec4 out_color;

uniform sampler2D u_texture;

void main()
{
	out_color = texture(u_texture, vs_out.uv);
}