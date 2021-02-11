// #include "../video/base/shader_buffers.hh"
// 
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/matrix_inverse.hpp>
// #include <glm/gtc/type_ptr.hpp>
// 
// 
// 
// using shape::video::cBaseShaderBuffers;
// 
// static glm::vec3 g_eye {0, 5, -5};
// static glm::vec3 g_top {0, 1, +0};
// 
// void render_cube(cBaseShaderBuffers& p_buffers, uint32_t p_frame)
// {
// 	auto l_model = p_buffers.get_lpmodel(p_frame);
// 	auto l_frame = p_buffers.get_lpframe(p_frame);
// 
// 	auto m = glm::identity<glm::mat4>();
// 	auto v = glm::lookAtLH(g_eye, {}, g_top);
// 	auto p = glm::perspectiveLH(glm::half_pi<float>(), 16.f / 9.f, 0.1f, 100.f);
// 
// 	auto mv  = m * v;
// 	auto vp  = v * p;
// 	auto mvp = mv * p;
// 
// 	auto im   = glm::inverseTranspose(m);
// 	auto iv   = glm::inverseTranspose(v);
// 	auto ip   = glm::inverseTranspose(p);
// 	auto imv  = glm::inverseTranspose(mv);
// 	auto ivp  = glm::inverseTranspose(vp);
// 	auto imvp = glm::inverseTranspose(mvp);
// 
// 	memcpy(l_model->model, glm::value_ptr(m), sizeof(glm::mat4));
// 	memcpy(l_model->model_view, glm::value_ptr(mv), sizeof(glm::mat4));
// 	memcpy(l_model->model_view_proj, glm::value_ptr(mvp), sizeof(glm::mat4));
// 
// 	memcpy(l_model->inv_model, glm::value_ptr(im), sizeof(glm::mat4));
// 	memcpy(l_model->inv_model_view, glm::value_ptr(imv), sizeof(glm::mat4));
// 	memcpy(l_model->inv_model_view_proj, glm::value_ptr(imvp), sizeof(glm::mat4));
// 
// 	memcpy(l_model->reference_top, glm::value_ptr(glm::vec4{ 0, 1, 0, 0 }), sizeof(glm::vec4));
// 	memcpy(l_model->reference_fwd, glm::value_ptr(glm::vec4{ 0, 0, 1, 0 }), sizeof(glm::vec4));
// 
// 	memcpy(l_frame->view, glm::value_ptr(v), sizeof(glm::mat4));
// 	memcpy(l_frame->proj, glm::value_ptr(p), sizeof(glm::mat4));
// 	memcpy(l_frame->view_proj, glm::value_ptr(vp), sizeof(glm::mat4));
// 
// 	memcpy(l_frame->inv_view, glm::value_ptr(iv), sizeof(glm::mat4));
// 	memcpy(l_frame->inv_proj, glm::value_ptr(ip), sizeof(glm::mat4));
// 	memcpy(l_frame->inv_view_proj, glm::value_ptr(ivp), sizeof(glm::mat4));
// }