// Tests to check integration and compilation of packages and libraries, separate from src files such that the tests are independent.
#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <tiny_obj_loader.h>
#include "ImGuiFileDialog.h"

static GLFWwindow* initGLFWContext() {
	REQUIRE(glfwInit() == GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	GLFWwindow* win = glfwCreateWindow(1, 1, "test", nullptr, nullptr);
	REQUIRE(win != nullptr);
	glfwMakeContextCurrent(win);
	REQUIRE(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == GLFW_TRUE);
	return win;
}

static void shutdownGLFWContext(GLFWwindow* win) {
	glfwDestroyWindow(win);
	glfwTerminate();
}

/* TEST CASES */

TEST_CASE("GLFW can initialise and shut down") {
	shutdownGLFWContext(initGLFWContext());
}

TEST_CASE("ImGui can initialise and shut down") {

	GLFWwindow* win = initGLFWContext();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(win, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	shutdownGLFWContext(win);
}

TEST_CASE("GLM vector can be constructed") {
	glm::vec3 v(1.0f, 1.0f, 1.0f);
	REQUIRE(v.x == 1.0f);
}

TEST_CASE("TinyOBJ material can be constructed") {
	tinyobj::material_t material;
	REQUIRE(material.name.empty());
}

TEST_CASE("ImGuiFileDialog can be constructed") {
	ImGuiFileDialog fileDialog;
	REQUIRE(true);
}

TEST_CASE("CRTDBG state comparison catches leak") {
#ifdef _DEBUG
	_CrtMemState startState;
	_CrtMemCheckpoint(&startState);
	int* leak = new int[10]; // intentional leak
	_CrtMemState endState, diffState;
	_CrtMemCheckpoint(&endState);
	bool leakDetected = _CrtMemDifference(&diffState, &startState, &endState);
	delete[] leak;
	REQUIRE(leakDetected);
#endif
#ifndef _DEBUG
	REQUIRE(true);
#endif
}

TEST_CASE("CRTDBG state comparison does not invent leak") {
	_CrtMemState startState;
	_CrtMemCheckpoint(&startState);
	_CrtMemState endState, diffState;
	_CrtMemCheckpoint(&endState);
	bool leakDetected = _CrtMemDifference(&diffState, &startState, &endState);
	REQUIRE(!leakDetected);
}