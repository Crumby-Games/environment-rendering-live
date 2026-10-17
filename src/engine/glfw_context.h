#pragma once
#include <string>
#include <functional>
#include "scene.h"

struct GLFWwindow;

// Handler for generic GLFW instance and application window. Links GLFW lifetime to its own.
// (Only one can exist concurrently)
class GLFWContext {
public:
	GLFWContext();
	~GLFWContext();
	// Create GLFWwindow and configure hints
	void constructWindow(const std::string& title, const int width, const int height, const std::vector<std::pair<int, int>>& hints);
	// Load OpenGL via GLAD
	void loadGL();
	// Update GLFW and window state. Called once per frame.
	// @return 0 if there are no issues and 1 if the window requests close.
	int update();
	// Time (in seconds) since last frame
	float frameDelta() noexcept { return delta_; }
	// Return underlying GLFWwindow
	GLFWwindow* window() noexcept { return window_; }
	// Width of the window in pixels
	int width() noexcept { return width_; }
	// Height of the window in pixels
	int height() noexcept { return height_; }
	// Horizontal multiplier based on OS scaling
	float scaleX() noexcept { return scaleX_; }
	// Vertical multiplier based on OS scaling
	float scaleY() noexcept { return scaleY_; }

	using ResizeCallback = std::function<void(int, int)>;
	using ContentScaleCallback = std::function<void(float, float)>;
	using MouseButtonCallback = std::function<void(int, int, int)>;
	using CursorCallback = std::function<void(double, double)>;
	using ScrollCallback = std::function<void(double, double)>;
	using KeyCallback = std::function<void(int, int, int)>;

	// Attach lambda that triggers when the window is resized. See glfwSetFramebufferSizeCallback().
	// @param int width. New width of window in pixels.
	// @param int height. New height of window in pixels.
	void onResize(bool invokeImmediately, ResizeCallback cb);

	// Attach lambda that triggers when the window is resized. See glfwSetWindowContentScaleCallback().
	// @param float scaleX. Factor of horizontal content scaling.
	// @param float scaleY. Factor of vertical content scaling.
	void onContentScaleChanged(bool invokeImmediately, ContentScaleCallback cb);

	// Attach lambda that triggers when the cursor is moved, including outside the window. See glfwSetCursorPosCallback()
	// @param int xpos. Position of cursor in the x axis in pixels. 0 is the left of window. Can be negative.
	// @param int ypos. Position of cursor in the y axis in pixels. 0 is the top of window. Can be negative.
	void onCursorMove(bool invokeImmediately, CursorCallback cb);

	// Attach lambda that triggers when any mouse button is pressed while the cursor is within the window. See glfwSetMouseButtonCallback()
	// @param int button. GLFW Enum. e.g. GLFW_MOUSE_BUTTON_LEFT
	// @param int state. GLFW Enum. e.g. GLFW_PRESSED
	// @param int mods. GLFW Flag. e.g. GLFW_MOD_SHIFT
	void onMouseButtonUpdate(MouseButtonCallback cb);

	// Attach lambda that triggers when the mouse wheel is scrolled while the cursor is within the window. See glfwSetScrollCallback()
	// @param int xoffset. Relative change in horizontal scrolling.
	// @param int yoffset. Relative change in vertical scrolling.
	void onMouseScroll(ScrollCallback cb);

	// Attach lambda that triggers when any keyboard button is pressed. See glfwSetScrollCallback() (scancode ignored)
	// @param int key. GLFW Enum. e.g. GLFW_KEY_R
	// @param int state. GLFW Enum. e.g. GLFW_PRESSED
	// @param int mods. GLFW Flag. e.g. GLFW_MOD_SHIFT
	void onKeyUpdate(KeyCallback cb);

	/* Disable copying and moving for safety */

	GLFWContext(GLFWContext&) = delete;
	GLFWContext& operator=(GLFWContext&) = delete;
private:
	void attachCallbacks();
	GLFWwindow* window_ = nullptr;
	bool minimised_ = false;
	float lastTime_;
	float delta_ = 1e-4f;
	int width_ = 0, height_ = 0;
	float scaleX_ = 1, scaleY_ = 1;
	ResizeCallback resize_;
	ContentScaleCallback contentScale_;
	MouseButtonCallback mouseButton_;
	CursorCallback cursor_;
	ScrollCallback scroll_;
	KeyCallback key_;
};
