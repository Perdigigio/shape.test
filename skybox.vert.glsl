#version 450

layout(location = 0) smooth out vec3 o_eye;

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

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	vec4 l_pos = vec4(0, 0, 0, 0);

	switch(gl_VertexID)
	{
		case 0: gl_Position = l_pos = vec4(-1, -1, 1, 1); break;
		case 1: gl_Position = l_pos = vec4(-1, +1, 1, 1); break;
		case 2: gl_Position = l_pos = vec4(+1, -1, 1, 1); break;
		case 3: gl_Position = l_pos = vec4(+1, +1, 1, 1); break;
	}

	o_eye = mat3(inv_view) * (inv_proj * l_pos).xyz;
}