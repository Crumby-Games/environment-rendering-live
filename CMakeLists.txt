cmake_minimum_required(VERSION 3.20)
project(RenderingSandbox VERSION 2.4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Catch2 CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)

add_subdirectory(external/ImGuiFileDialog)

# Project

add_library(RenderingSandboxLib)
target_sources(RenderingSandboxLib
	PRIVATE
		src/app/demo_app.h
		src/app/demo_app_main.cpp
		src/app/demo_app_gui.cpp
		src/app/demo_scene.cpp
		src/app/demo_scene.h
		src/app/environment.cpp
		src/app/environment.h
		src/app/input.h
		src/app/main.cpp
		src/app/orbit_camera.cpp
		src/app/orbit_camera.h
		src/app/gui/helpers.cpp
		src/app/gui/helpers.h
		src/app/gui/layout.cpp
		src/app/gui/layout.h
		src/app/gui/widgets.h

		src/backend/gl/attachment.h
		src/backend/gl/dbg.h
		src/backend/gl/framebuffer.cpp
		src/backend/gl/framebuffer.h
		src/backend/gl/resource.h
		src/backend/gl/renderer.cpp
		src/backend/gl/renderer.h
		src/backend/gl/shader.cpp
		src/backend/gl/shader.h
		src/backend/gl/texture.cpp
		src/backend/gl/texture.h
		src/backend/gl/vertex_objects.cpp
		src/backend/gl/vertex_objects.h

		src/core/type_utils.h

		src/engine/application.h
		src/engine/camera.h
		src/engine/glfw_context.cpp
		src/engine/glfw_context.h
		src/engine/image.cpp
		src/engine/image.h
		src/engine/mesh.cpp
		src/engine/mesh.h
		src/engine/model.h
		src/engine/pass.cpp
		src/engine/pass.h
		src/engine/pipeline.h
		src/engine/pipeline.cpp
		src/engine/scene.cpp
		src/engine/scene.h
		src/engine/shader_material.cpp
		src/engine/shader_material.h
)

target_include_directories(RenderingSandboxLib PUBLIC src)

target_compile_definitions(RenderingSandboxLib PUBLIC 
	RESOURCE_DIR="${CMAKE_SOURCE_DIR}/resources/"
	MESH_DIR="${CMAKE_SOURCE_DIR}/resources/meshes/"
	IMG_DIR="${CMAKE_SOURCE_DIR}/resources/images/"
	SHADER_DIR="${CMAKE_SOURCE_DIR}/resources/shaders/"
)

target_link_libraries(RenderingSandboxLib
	PUBLIC
		glm::glm
		ImGuiFileDialog
	PRIVATE
		tinyobjloader::tinyobjloader
		glfw
		glad::glad  
		imgui::imgui
)

# Application

add_executable(RenderingSandboxApp src/app/main.cpp)
target_link_libraries(RenderingSandboxApp PRIVATE RenderingSandboxLib)
target_compile_definitions(RenderingSandboxApp PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Tests

include(Catch)

add_executable(RenderingSandboxTests
	tests/test_libraries.cpp
	tests/test_mesh.cpp
	tests/test_image.cpp
	tests/test_orbit_camera.cpp
)

target_link_libraries(RenderingSandboxTests PRIVATE Catch2::Catch2WithMain RenderingSandboxLib)
catch_discover_tests(RenderingSandboxTests)
