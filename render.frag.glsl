#version 450

layout(location = 0) in vec4 f_pos;
layout(location = 1) in vec4 f_nor;
layout(location = 2) in vec2 f_tex;
layout(location = 3) in vec3 f_env;

layout(location = 0) out vec4 o_albedo;
layout(location = 1) out vec4 o_normal;
layout(location = 2) out vec4 o_params;
layout(location = 3) out float o_fdepth;

layout(binding = 0) uniform sampler2D s_dist;

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

layout(binding = 1, std140) uniform per_frame
{
	mat4 view;
	mat4 proj;
	mat4 view_proj;
	mat4 inv_view;
	mat4 inv_proj;
	mat4 inv_view_proj;

	vec4 camera_pos;
	vec4 camera_fwd;
	vec4 camera_top;
	vec2 viewport_pos;
	vec2 viewport_ext;

	vec4 l_dir;
	vec4 l_col;
};

//	float h(float t)
//	{
//		return t * t * (3.0 - (2.0 * t));
//	}
//
//	float ss(float a, float b, float x)
//	{
//		return h(clamp((x - a) / (b - a), 0.0, 1.0));
//	}

const float PI = 3.14159;

void main()
{
	vec3 V = normalize(f_pos.xyz - camera_pos.xyz);
	vec3 N = normalize(f_nor.xyz);
	vec3 L = normalize(l_dir.xyz);

	vec3 H = normalize(V + L);

	float NL = max(dot(N, L), 0);
	float NH = max(dot(N, H), 0);
	float VN = max(dot(V, N), 0);
	float VH = max(dot(V, H), 0);


	float m = 0.1;
	float D = texture(s_dist, vec2(NH, m)).r; // BECKMAN
	float F = params.x + (1 - params.x) * pow(1 - NL, 5);
	float G = 1;

	G = min(G, 2 * NH * VN / VH);
	G = min(G, 2 * NH * NL / VH);

	float Ks = (D * F * G) / (VN * NL * PI);
	float Kd = NL;

	o_albedo = 0.2 * albedo + 0.8 * (clamp(Ks, 0, 1) + Kd * albedo);
	o_normal = vec4((f_nor.xy + 1) / 2, 0, 0);
	o_params = vec4(F, D, NH, 0);

	o_fdepth = gl_FragCoord.w / gl_FragCoord.z;
}