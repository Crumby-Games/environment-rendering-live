#pragma once
#include <vector>
#include <array>
#include "vertex_objects.h"

// Bridge between a VAO and its VBOs
class GLRenderer {
public:
	GLRenderer(const int width, const int height) : viewport_({ 0, 0, width,height }) {}
	// Uploads vertex data to GPU in order of subvectors.
	// In other words, the 0th vector becomes vertex attribute 0, 1st vector becomes vertex attribute 1, etc.
	// The 0th attribute will be used to determine the vertex count.
	void uploadAttributes(const std::vector<std::vector<float>>& attributeStreams);
	void prepare() const noexcept;

	// Bind VAO and draw triangles
	void render() const;

	// Whether it has any VBOs.
	bool hasData() const noexcept { return !buffers_.empty(); }

	bool& depthTesting() noexcept { return depthTest_; }
	bool depthTesting() const noexcept { return depthTest_; }

	bool& wireframe() noexcept { return wireframe_; }
	bool wireframe() const noexcept { return wireframe_; }

	std::array<int, 4>& viewport() noexcept { return viewport_; }
	const std::array<int, 4>& viewport() const noexcept { return viewport_; }

	std::array<float, 4>& clearColor() noexcept { return clearColor_; }
	const std::array<float, 4>& clearColor() const noexcept { return clearColor_; }

	// Disable copy assignment and copy construction because this class uses GLResources
	GLRenderer(GLRenderer&) = delete;
	GLRenderer& operator=(GLRenderer&) = delete;
	GLRenderer(GLRenderer&&) noexcept = default;
	GLRenderer& operator=(GLRenderer&&) noexcept = default;
private:
	size_t vertexCount_ = 0;
	GLVertexArray vao_;
	std::vector<GLBuffer> buffers_;

	bool depthTest_ = false;
	bool wireframe_ = false;
	std::array<int, 4> viewport_;
	std::array<float, 4> clearColor_{ 0,0,0,0 };
};
