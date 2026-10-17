#include "glfw_context.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>

GLFWContext::GLFWContext() {
	if (!glfwInit())
		throw std::runtime_error("Failed to initialise GLFW");
	lastTime_ = glfwGetTime();
}

GLFWContext::~GLFWContext() {
	if (window_) glfwDestroyWindow(window_);
	glfwTerminate();
}

void GLFWContext::constructWindow(const std::string& title, const int width, const int height, const std::vector<std::pair<int, int>>& hints) {
	if (window_) return;
	for (auto& [k, v] : hints) {
		glfwWindowHint(k, v);
	}

	window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr); // Width and height are scaled automatically by OS scaling

	if (!window_)
		throw std::runtime_error("Failed to create GLFW window");

	glfwMakeContextCurrent(window_);
	attachCallbacks();
	glfwGetWindowSize(window_, &width_, &height_); // Width and height after OS scaling
	glfwGetWindowContentScale(window_, &scaleX_, &scaleY_);
}

void GLFWContext::loadGL() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Failed to initialise GLAD");
}

void GLFWContext::attachCallbacks() {
	if (!window_) return;
	glfwSetWindowUserPointer(window_, this);

	glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* win, int width, int height)
		{
			auto* self = static_cast<GLFWContext*>(glfwGetWindowUserPointer(win));
			self->width_ = width; self->height_ = height;
			if (self->resize_) self->resize_(width, height);
		}
	);

	glfwSetWindowContentScaleCallback(window_, [](GLFWwindow* win, float scaleX, float scaleY)
		{
			auto* self = static_cast<GLFWContext*>(glfwGetWindowUserPointer(win));
			self->scaleX_ = scaleX; self->scaleY_ = scaleY;
			if (self->contentScale_) self->contentScale_(scaleX, scaleY);
		}
	);

	glfwSetMouseButtonCallback(window_, [](GLFWwindow* win, int button, int state, int mods)
		{
			auto* self = static_cast<GLFWContext*>(glfwGetWindowUserPointer(win));
			if (self->mouseButton_) self->mouseButton_(button, state, mods);
		}
	);

	glfwSetCursorPosCallback(window_, [](GLFWwindow* win, double xpos, double ypos)
		{
			auto* self = static_cast<GLFWContext*>(glfwGetWindowUserPointer(win));
			if (self->cursor_) self->cursor_(xpos, ypos);
		}
	);

	glfwSetScrollCallback(window_, [](GLFWwindow* win, double xoffset, double yoffset)
		{
			auto* self = static_cast<GLFWContext*>(glfwGetWindowUserPointer(win));
			if (self->scroll_) self->scroll_(xoffset, yoffset);
		}
	);

	glfwSetKeyCallback(window_, [](GLFWwindow* win, int key, int scancode, int state, int mods)
		{
			auto* self = static_cast<GLFWContext*>(glfwGetWindowUserPointer(win));
			if (self->key_) self->key_(key, state, mods);
		}
	);
}

int GLFWContext::update() {
	if (!window_) return 2;

	float t = glfwGetTime();
	delta_ = t - lastTime_;
	lastTime_ = t;

	int width, height;

	// Trigger callbacks
	glfwPollEvents();

	// Get window size to check if it's minimised
	glfwGetFramebufferSize(window_, &width, &height);
	// If minimised, wait until no longer minimised or the window requests close otherwise
	while ((width == 0 || height == 0) && !glfwWindowShouldClose(window_)) {
		glfwWaitEvents();
		glfwGetFramebufferSize(window_, &width, &height);
	}

	// If the window requests close, return different state and don't update screen
	if (glfwWindowShouldClose(window_)) return 1;

	// Update screen
	glfwSwapBuffers(window_);

	return 0;
}

void GLFWContext::onResize(bool invokeImmediately, ResizeCallback cb) {
	resize_ = std::move(cb);
	if (invokeImmediately) {
		int w, h;
		glfwGetWindowSize(window_, &w, &h);
		resize_(w, h);
	}
}

void GLFWContext::onContentScaleChanged(bool invokeImmediately, ContentScaleCallback cb) {
	contentScale_ = std::move(cb);
	if (invokeImmediately) {
		float x, y;
		glfwGetWindowContentScale(window_, &x, &y);
		contentScale_(x, y);
	}
}

void GLFWContext::onCursorMove(bool invokeImmediately, CursorCallback cb) {
	cursor_ = std::move(cb);
	if (invokeImmediately) {
		double x, y;
		glfwGetCursorPos(window_, &x, &y);
		cursor_(x, y);
	}
}

void GLFWContext::onMouseButtonUpdate(MouseButtonCallback cb) { mouseButton_ = std::move(cb); }

void GLFWContext::onMouseScroll(ScrollCallback cb) { scroll_ = std::move(cb); }

void GLFWContext::onKeyUpdate(KeyCallback cb) { key_ = std::move(cb); }
