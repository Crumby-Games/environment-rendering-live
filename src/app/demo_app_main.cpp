#include "demo_app.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <imgui.h>
#include <iostream>

DemoApplication::DemoApplication() : Application() {
	glfw().constructWindow("Rendering Sandbox", 900, 640, {
		{ GLFW_CONTEXT_VERSION_MAJOR, 4 },
		{ GLFW_CONTEXT_VERSION_MINOR, 6 },
		{ GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE },
		{ GLFW_SCALE_TO_MONITOR, GLFW_TRUE }
		}
	);
	glfw().loadGL();

	try {
		instantiateScene<DemoScene>(900 - sidebarWidth_, 640);
		demoScene()->changeMainModel(MESH_DIR + std::string("tests/cube.obj"), false);
		demoScene()->connectInput(input_);
	}
	catch (const std::exception& e) {
		std::cout << "Error occured during Scene instantiation. Message:\n" << e.what() << std::endl;
	}

	glfw().onResize(true, [this](int width, int height)
		{
			if (width == 0 || height == 0) return;
			scene()->resizeViewport(width - sidebarWidth_, height);
		}
	);

	gui_init();
	configureInput();
}

DemoApplication::~DemoApplication() {
	gui_shutdown();
}

int DemoApplication::run() {
	try {
		// Runs once per frame
		while (glfw().update() == 0) {
			gui_newFrame();

			// 1. Build UI
			gui_build();

			// 2. Update Scene
			scene()->process(glfw().frameDelta());

			// 3. Render Scene
			scene()->render();

			// 4. Render UI
			gui_render();

			// Reset per-frame accumulating input data
			input_.mouse.dx = input_.mouse.dy = input_.mouse.dw = 0.0;
		}
	}
	catch (const std::exception& e) {
		std::cout << "Error during run(). Message:\n" << e.what() << std::endl;
		return 1;
	}
	return 0;
}

// Takes a GLFW_MOUSE_BUTTON constant and converts it to a MouseButton
static MouseButton MouseButtonFromGLFW(int button) {
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:   return MouseButton::Left;
	case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
	case GLFW_MOUSE_BUTTON_RIGHT:  return MouseButton::Right;
	default:					   return MouseButton::None;
	}
};

void DemoApplication::configureInput() {
	glfw().onMouseButtonUpdate([this](int button_, int state, int mods)
		{
			if (ImGui::GetIO().WantCaptureMouse) return;

			auto button = MouseButtonFromGLFW(button_);
			Input::Mouse& mouse = input_.mouse;

			if (button == MouseButton::None) return;

			if (state == GLFW_PRESS) {
				if (mouse.activeButton != MouseButton::None) return;

				mouse.activeButton = button;
				mouse.dx = mouse.dy = mouse.dw = 0.0;
			}
			else if (mouse.activeButton == button) mouse.activeButton = MouseButton::None;
		}
	);

	glfw().onCursorMove(true, [this](double xpos, double ypos)
		{
			if (ImGui::GetIO().WantCaptureMouse) return;
			Input::Mouse& mouse = input_.mouse;

			if (mouse.x != 0.0 || mouse.y != 0.0) {
				mouse.dx += mouse.x - xpos;
				mouse.dy += mouse.y - ypos;
			}
			mouse.x = xpos;
			mouse.y = ypos;
		});

	glfw().onMouseScroll([this](double xoffset, double yoffset)
		{
			if (ImGui::GetIO().WantCaptureMouse) return;
			input_.mouse.dw += yoffset;
		}
	);
}
