#version 450

layout(location = 0) in vec3 f_eye;

layout(location = 0) out vec4 o_albedo;
layout(location = 1) out vec4 o_normal;
layout(location = 2) out vec4 o_params;

layout(binding = 1) uniform samplerCube s_cube;

void main()
{
	o_albedo = texture(s_cube, f_eye);
}