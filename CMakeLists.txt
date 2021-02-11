cmake_minimum_required(VERSION 3.0)

project(shape.test VERSION 1.0.0 LANGUAGES C CXX)

add_subdirectory(modules/core)
add_subdirectory(modules/glfw)

add_executable(test test.cc)

target_link_libraries(test PRIVATE shape_core shape_core_video)
target_link_libraries(test PRIVATE glfw)

configure_file(render.vert.glsl render.vert.glsl COPYONLY)
configure_file(render.frag.glsl render.frag.glsl COPYONLY)
configure_file(skybox.vert.glsl skybox.vert.glsl COPYONLY)
configure_file(skybox.frag.glsl skybox.frag.glsl COPYONLY)
configure_file(data/x+.bmp data/x+.bmp COPYONLY)
configure_file(data/x-.bmp data/x-.bmp COPYONLY)
configure_file(data/y+.bmp data/y+.bmp COPYONLY)
configure_file(data/y-.bmp data/y-.bmp COPYONLY)
configure_file(data/z+.bmp data/z+.bmp COPYONLY)
configure_file(data/z-.bmp data/z-.bmp COPYONLY)
configure_file(data/cube.obj data/cube.obj COPYONLY)
