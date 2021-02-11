#version 450

layout(location = 0) in vec3 v_pos;

layout(location = 1) in vec4 v_nor_0;
layout(location = 2) in vec4 v_nor_1;
layout(location = 3) in vec2 v_tex_0;
layout(location = 4) in vec2 v_tex_1;
layout(location = 5) in vec4 v_skn_0;
layout(location = 6) in vec4 v_skn_1;

layout(location = 0) out vec4 o_pos;
layout(location = 1) out vec4 o_nor;
layout(location = 2) out vec2 o_tex;
layout(location = 3) out vec2 o_env;

layout(binding = 0, std140) uniform per_model
{
	mat4 model;
	mat4 model_view;
	mat4 model_view_proj;

	mat4 inv_model;
	mat4 inv_model_view;
	mat4 inv_model_view_proj;

	vec4 reference_pos;
	vec4 reference_fwd;
	vec4 reference_top;
	vec4 albedo;
	vec4 normal;
	vec4 params;
	mat2 transform_uv0;
	mat2 transform_uv1;
};

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	vec3 l_nor = v_nor_0.xyz * 2 - 1;

	o_pos = model * vec4(v_pos, 1);
	o_nor = model * vec4(l_nor, 0);
	o_tex = v_tex_0;

	//
	//

	gl_Position = model_view_proj * vec4(v_pos, 1);
}