//#include <video/glad/glad_wgl.h>

#include <base_camera.hh>
#include <base_image.hh>
#include <base_image_bmp.hh>
#include <base_image_tga.hh>
#include <base_model.hh>
#include <base_model_obj.hh>

#include <video_gl_base_device.hh>
#include <video_gl_base_render_buffers.hh>
#include <video_gl_base_shader.hh>
#include <video_gl_base_shader_program.hh>
#include <video_gl_base_texture.hh>
#include <video_gl_base_model.hh>
#include <video_gl_base_model_params.hh>
#include <video_gl_base_model_vertex.hh>

#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <chrono>
#include <cstring>

#include <math_affine.hh>
#include <math_quaternion.hh>

#include <GLFW/glfw3.h>

#define WINDOW_X CW_USEDEFAULT
#define WINDOW_Y CW_USEDEFAULT
#define WINDOW_W 1000
#define WINDOW_H 1000

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void load_buffer(shape::video::base_model_buffer *, float);
static void load_buffer(shape::video::base_frame_buffer *, float);

static void load_distribution(shape::video::cBaseTexture &, uint32_t, uint32_t);

static std::vector<char> read_file(const char *p_file)
{
	std::ifstream l_file{ p_file };

	auto l_vec = std::vector<char>{ std::istream_iterator<char>(l_file >> std::noskipws), std::istream_iterator<char>() };

	l_vec.push_back(0);

	return l_vec;
}

shape::cBaseCamera g_cam { 0.2f, 0.2f, 0.1f, 400.f };

// shape::real3 g_pos = { 0, 5, 0 };
// shape::real3 g_top = { 0, 1, 0 };
// shape::real3 g_fwd = { 0, 0, 1 };

// void move_W(float dt) noexcept { g_pos = g_pos + g_fwd * 10.f * dt; }
// void move_S(float dt) noexcept { g_pos = g_pos - g_fwd * 10.f * dt; }
// void move_D(float dt) noexcept { g_pos = g_pos + shape::cross(g_top, g_fwd) * 10.f * dt; }
// void move_A(float dt) noexcept { g_pos = g_pos - shape::cross(g_top, g_fwd) * 10.f * dt; }

void move_W(float dt) noexcept { g_cam.set_view_pos(g_cam.get_view_pos() + g_cam.get_view_fwd() * 10.f * dt); }
void move_S(float dt) noexcept { g_cam.set_view_pos(g_cam.get_view_pos() - g_cam.get_view_fwd() * 10.f * dt); }
void move_D(float dt) noexcept { g_cam.set_view_pos(g_cam.get_view_pos() + shape::cross(g_cam.get_view_top(), g_cam.get_view_fwd()) * 10.f * dt); }
void move_A(float dt) noexcept { g_cam.set_view_pos(g_cam.get_view_pos() - shape::cross(g_cam.get_view_top(), g_cam.get_view_fwd()) * 10.f * dt); }

auto g_t1 = std::chrono::steady_clock::now();
auto g_t2 = std::chrono::steady_clock::now();

bool g_W = false;
bool g_A = false;
bool g_S = false;
bool g_D = false;
bool g_B = false;

inline std::ostream& operator << (std::ostream &p_stream, shape::real4 v)
{
	return p_stream << '[' 
		<< shape::x(v) << ' '
		<< shape::y(v) << ' '
		<< shape::z(v) << ' '
		<< shape::w(v) << ']' << std::endl;
}

int main()
{
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

//	#ifdef DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
//	#endif

	auto l_hwnd = glfwCreateWindow(WINDOW_W, WINDOW_H, "shape.test", NULL, NULL);

	if (!l_hwnd)
	{
		glfwTerminate();

		return LOG_FAIL("Cannot open window"), EXIT_FAILURE;
	}

	glfwSetKeyCallback(l_hwnd, [](GLFWwindow* w, int k, int s, int a, int m)
	{
		switch (k)
		{
			case GLFW_KEY_W: g_W = (a != GLFW_RELEASE); break;
			case GLFW_KEY_A: g_A = (a != GLFW_RELEASE); break;
			case GLFW_KEY_S: g_S = (a != GLFW_RELEASE); break;
			case GLFW_KEY_D: g_D = (a != GLFW_RELEASE); break;
		}
	});

	glfwMakeContextCurrent(l_hwnd);

	shape::video::cBaseDevice{}.init();

	glDebugMessageCallback([](GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
	{
		LOG_INFO("GL: " << message);
	}, NULL);

	shape::video::cBaseModel l_video_model;
	{
		const auto l_mod = shape::base_model_obj::load_temp({ shape::cBaseStream::ISTREAM, "../data/cube.obj" });

		GLuint l_fmt =
			(l_mod.has_pos() ? shape::video::base_model_flag::has_pos : 0) |
			(l_mod.has_nor() ? shape::video::base_model_flag::has_nor : 0) |
			(l_mod.has_tex() ? shape::video::base_model_flag::has_tex : 0) |
			(l_mod.has_skn() ? shape::video::base_model_flag::has_skn : 0);

		l_video_model = shape::video::cBaseModel
		{
			l_mod.get_vtx(), l_mod.get_idx(), l_fmt
		};

		l_video_model.update_pos(0, l_mod.get_vtx(), l_mod.get_pos_buffer());
		l_video_model.update_nor(0, l_mod.get_vtx(), l_mod.get_nor_buffer());
		l_video_model.update_tex(0, l_mod.get_vtx(), l_mod.get_tex_buffer());
		l_video_model.update_idx(0, l_mod.get_idx(), l_mod.get_idx_buffer());
	}


	shape::video::cBaseModelVertex l_empty_vertex{ 0 };
	shape::video::cBaseModelVertex l_model_vertex
	{
		shape::video::base_model_flag::has_pos |
		shape::video::base_model_flag::has_nor |
		shape::video::base_model_flag::has_tex
	};

	shape::video::cBaseShader l_render_vshader{ shape::video::cBaseShader::VSHADER, read_file("../render.vert.glsl").data() };
	shape::video::cBaseShader l_render_fshader{ shape::video::cBaseShader::FSHADER, read_file("../render.frag.glsl").data() };
	shape::video::cBaseShader l_skybox_vshader{ shape::video::cBaseShader::VSHADER, read_file("../skybox.vert.glsl").data() };
	shape::video::cBaseShader l_skybox_fshader{ shape::video::cBaseShader::FSHADER, read_file("../skybox.frag.glsl").data() };

	//auto l_vshader = shape::video::base_shader::init_vshader(g_vshader);
	//auto l_fshader = shape::video::base_shader::init_fshader(g_fshader);

	shape::video::cBaseShaderProgram l_render_program{ shape::video::cBaseShaderProgram::INIT };
	shape::video::cBaseShaderProgram l_skybox_program{ shape::video::cBaseShaderProgram::INIT };

	l_render_program.bind_vshader(l_render_vshader);
	l_render_program.bind_fshader(l_render_fshader);
	l_skybox_program.bind_vshader(l_skybox_vshader);
	l_skybox_program.bind_fshader(l_skybox_fshader);

	shape::video::cBaseModelParams l_model_params{ 3, 3, 0 };

	LOG_INFO("model: " << l_model_params.get_model_length());
	LOG_INFO("frame: " << l_model_params.get_frame_length());
	LOG_INFO("blend: " << l_model_params.get_blend_length());

	shape::video::cBaseRenderBuffers l_w{ };
	shape::video::cBaseRenderBuffers l_b{ WINDOW_W / 2, WINDOW_H / 2, 8 };

	shape::video::cBaseTexture l_tex_dist{ shape::video::cBaseTexture::TEXTURE_2D, shape::video::pixel::FLT1, 1, { 256, 512 }};

	glTextureParameteri(l_tex_dist.get_handle(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(l_tex_dist.get_handle(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(l_tex_dist.get_handle(), GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(l_tex_dist.get_handle(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(l_tex_dist.get_handle(), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameterf(l_tex_dist.get_handle(), GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);

	load_distribution(l_tex_dist, 256, 512);

	shape::video::cBaseTexture l_tex_cube{ shape::video::cBaseTexture::CUBEMAP, shape::video::pixel::BM32, 1, { 512, 512 }};
	{
		shape::cBaseImageData<shape::temp_t> l_img[] =
		{
			shape::base_image_bmp::load_temp({ shape::cBaseStream::ISTREAM, "../data/x+.bmp" }),
			shape::base_image_bmp::load_temp({ shape::cBaseStream::ISTREAM, "../data/x-.bmp" }),
			shape::base_image_bmp::load_temp({ shape::cBaseStream::ISTREAM, "../data/y+.bmp" }),
			shape::base_image_bmp::load_temp({ shape::cBaseStream::ISTREAM, "../data/y-.bmp" }),
			shape::base_image_bmp::load_temp({ shape::cBaseStream::ISTREAM, "../data/z+.bmp" }),
			shape::base_image_bmp::load_temp({ shape::cBaseStream::ISTREAM, "../data/z-.bmp" })
		};

		l_tex_cube.update_3d(0, l_img[0].get_bitmap(), shape::video::pixel::BM24, { 0, 512, 0, 512, 0, 1 }); //! +X
		l_tex_cube.update_3d(0, l_img[1].get_bitmap(), shape::video::pixel::BM24, { 0, 512, 0, 512, 1, 2 }); //! -X
		l_tex_cube.update_3d(0, l_img[2].get_bitmap(), shape::video::pixel::BM24, { 0, 512, 0, 512, 2, 3 }); //! +Y
		l_tex_cube.update_3d(0, l_img[3].get_bitmap(), shape::video::pixel::BM24, { 0, 512, 0, 512, 3, 4 }); //! -Y
		l_tex_cube.update_3d(0, l_img[4].get_bitmap(), shape::video::pixel::BM24, { 0, 512, 0, 512, 4, 5 }); //! +Z
		l_tex_cube.update_3d(0, l_img[5].get_bitmap(), shape::video::pixel::BM24, { 0, 512, 0, 512, 5, 6 }); //! -Z
	}

	//!
	//! TEST HACK
	//!

	glFrontFace(GL_CW);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_FRAMEBUFFER_SRGB);


	auto l_frame = 0U;

	shape::video::base_model_buffer l_model_buffer;
	shape::video::base_frame_buffer l_frame_buffer;

	double l_cur_new[2] = {};
	double l_cur_old[2] = {};

	do
	{
		std::chrono::duration<float> dt = g_t2 - g_t1;

		g_t1 = g_t2;
		g_t2 = std::chrono::steady_clock::now();

		if (g_W) move_W(dt.count());
		if (g_A) move_A(dt.count());
		if (g_S) move_S(dt.count());
		if (g_D) move_D(dt.count());

		l_cur_old[0] = l_cur_new[0];
		l_cur_old[1] = l_cur_new[1];

		glfwGetCursorPos(l_hwnd, &l_cur_new[0], &l_cur_new[1]);

		if (glfwGetMouseButton(l_hwnd, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			float dx = (float)(l_cur_new[0] - l_cur_old[0]) * dt.count();
			float dy = (float)(l_cur_new[1] - l_cur_old[1]) * dt.count();

			g_cam.rotate_y(dx);
			//g_cam.rotate_x(dy);

			//shape::q_rotate_y(g_fwd, dx, g_fwd);
		}

		l_b.clear_albedo(0.39f, 0.58f, 0.93f, 1.f); //! CORNFLOWER BLUE
		l_b.clear_normal(0.39f, 0.58f, 0.93f, 1.f); //! CORNFLOWER BLUE
		l_b.clear_params(0.39f, 0.58f, 0.93f, 1.f); //! CORNFLOWER BLUE
		l_b.clear_tdepth(1);

		if (l_frame == 0)
		{
			l_model_params.discard_model();
			l_model_params.discard_frame();
		}

		glViewport(0, 0, WINDOW_W / 2, WINDOW_H / 2);

		glBindTextureUnit(0, l_tex_dist.get_handle());
		glBindTextureUnit(1, l_tex_cube.get_handle());

		l_b.bind_for_draw();

		load_buffer(&l_model_buffer, dt.count());
		load_buffer(&l_frame_buffer, dt.count());

		l_model_params.copy_model(&l_model_buffer);
		l_model_params.copy_frame(&l_frame_buffer);
		l_model_params.bind_model(0);
		l_model_params.bind_frame(1);

		//!
		//! CUBE
		//!

		glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

		l_render_program.bind();

		l_model_vertex.bind();
			l_model_vertex.attach_pos(l_video_model.get_pos_buffer(), 0);
			l_model_vertex.attach_nor(l_video_model.get_nor_buffer(), 0);
			l_model_vertex.attach_tex(l_video_model.get_tex_buffer(), 0);
			l_model_vertex.attach_idx(l_video_model.get_idx_buffer());

		glDrawElements(GL_TRIANGLES, l_video_model.get_idx(), GL_UNSIGNED_INT, NULL);

		l_skybox_program.bind();

		l_empty_vertex.bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		l_b.bind_for_read();
		l_w.bind_for_draw();

		auto __x = (WINDOW_W / 2);
		auto __y = (WINDOW_H / 2);

		glReadBuffer( GL_COLOR_ATTACHMENT0);
			glBlitFramebuffer(0, 0, WINDOW_W / 2, WINDOW_H / 2, 0 * __x, 0 * __y, 0 * __x + __x, 0 * __y + __y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glReadBuffer( GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(0, 0, WINDOW_W / 2, WINDOW_H / 2, 0 * __x, 1 * __y, 0 * __x + __x, 1 * __y + __y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glReadBuffer( GL_COLOR_ATTACHMENT2);
			glBlitFramebuffer(0, 0, WINDOW_W / 2, WINDOW_H / 2, 1 * __x, 1 * __y, 1 * __x + __x, 1 * __y + __y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		l_b.discard_tdepth();
		l_b.discard_albedo();

		glFlush();
		glfwSwapBuffers(l_hwnd);
		glfwPollEvents();

		l_frame += 1;
		l_frame %= 3;

		l_model_params.next_model();
		l_model_params.next_frame();

	} while(!glfwWindowShouldClose(l_hwnd));



	return 0;
}

//void load_proj(shape::real4x4 &m, float w, float h, float n, float f) noexcept
//{
//	float a = 2 * n / w;
//	float b = 2 * n / h;
//	float c = f / (f - n);
//	float d = f / (n - f) * n;
//
//	//!
//	//!
//
//	shape::set<float>(shape::r1(m), a, 0, 0, 0);
//	shape::set<float>(shape::r2(m), 0, b, 0, 0);
//	shape::set<float>(shape::r3(m), 0, 0, c, d);
//	shape::set<float>(shape::r4(m), 0, 0, 1, 0);
//}
//
//shape::real4x4& load_view(shape::real4x4 &v) noexcept
//{
//	const shape::real3 l_rgt = shape::cross(g_top, g_fwd);
//
//	shape::set(shape::r1(v), l_rgt, 0.f);
//	shape::set(shape::r2(v), g_top, 0.f);
//	shape::set(shape::r3(v), g_fwd, 0.f);
//	shape::set(shape::r4(v), g_pos, 1.f);
//
//	//!
//	//!
//
//	return shape::transpose(v, shape::invert(v));
//}

static float PI = 3.14159f;

static void load_buffer(shape::video::base_model_buffer *p_model, float t)
{
	shape::real4x4 m;
	shape::real4x4 v = g_cam.get_view();
	shape::real4x4 p = g_cam.get_proj();
	shape::real4x4 a;
	shape::real4x4 b;

	shape::load_identity(m);

	//load_view(v);
	//load_proj(p, 0.2f, 0.2f, 0.1f, 400.f);

	a = shape::mul(v, m);
	b = shape::mul(p, a);

	shape::transpose(p_model->model, m);
	shape::transpose(p_model->model_view, a);
	shape::transpose(p_model->model_view_proj, b);

	shape::invert(p_model->inv_model, m);
	shape::invert(p_model->inv_model_view, a);
	shape::invert(p_model->inv_model_view_proj, b);

	float n1 = (1.f);
	float n2 = (3.f);
	float R0 = pow((n1 - n2) / (n1 + n2), 2);

	shape::copy(p_model->albedo, { 0.5f, 0.3f, 0.5f, 0.0f });
	shape::copy(p_model->normal, { 1.0f, 1.0f, 1.0f, 0.0f });
	shape::copy(p_model->params, { R0, 0.0f, 0.0f, 0.0f });

}

static void load_buffer(shape::video::base_frame_buffer *p_frame, float t)
{
	static shape::real4 light_dir = shape::normalize(shape::real4{ 0, 0, -1, 0 });
	static shape::real4 light_col = { 1, 1, 1, 1 };

	shape::real4x4 v = g_cam.get_view();
	shape::real4x4 p = g_cam.get_proj();

	//load_view(v);
	//load_proj(p, 0.2f, 0.2f, 0.1f, 400.f);

	shape::transpose(p_frame->view, v);
	shape::transpose(p_frame->proj, p);
	shape::transpose(p_frame->inv_view, shape::invert(v));
	shape::transpose(p_frame->inv_proj, shape::invert(p));

	shape::mul(p_frame->camera_pos, v, { 0, 0, 0, 1 });
	shape::mul(p_frame->camera_top, v, { 0, 1, 0, 0 });
	shape::mul(p_frame->camera_fwd, v, { 0, 0, 1, 0 });

	shape::copy(p_frame->light_dir, light_dir);
	shape::copy(p_frame->light_col, light_col);
}

#include <algorithm>

void load_distribution(shape::video::cBaseTexture &p_tex, uint32_t w, uint32_t h)
{
	std::vector<float> l_tex(w * h, 0);

	for (uint32_t i = 0; i < h; i++)
	for (uint32_t j = 0; j < w; j++)
	{
		float m = std::max(i / float(h), 1.f / h);
		float v = std::max(j / float(w), 1.f / w);
		float t = 1 / (v * v) - 1;

		l_tex[i * w + j] = std::exp(-t / (m * m)) / (PI * m * m * v * v * v * v);

		//if (j != 0)
		//	l_tex[i * w + j] = (m * m) / (PI * pow(v * v * (m * m + t * t), 2));
	}

	p_tex.update_2d(0, l_tex.data(), shape::video::pixel::FLT1);
}