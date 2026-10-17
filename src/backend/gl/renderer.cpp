#include "renderer.h"
#include <glad/glad.h>
#include <iostream>
#include "dbg.h"

void GLRenderer::uploadAttributes(const std::vector<std::vector<float>>& attributeStreams) {
	buffers_.clear();
	vao_ = GLVertexArray();

	vao_.bind();
	int i = 0;
	for (const auto& attributeStream : attributeStreams) {
		buffers_.emplace_back(attributeStream);
		vao_.setAttrib(i++, buffers_.back());
	}

	vertexCount_ = attributeStreams.front().size() / 3;
}

void GLRenderer::render() const {
	if (vertexCount_ == 0) return;
	vao_.bind();
	GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, vertexCount_));
}

void GLRenderer::prepare() const noexcept {
	if (depthTest_) glEnable(GL_DEPTH_TEST);
	else glDisable(GL_DEPTH_TEST);
	if (wireframe_) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glViewport(viewport_[0], viewport_[1], viewport_[2], viewport_[3]);
	glClearColor(clearColor_[0], clearColor_[1], clearColor_[2], clearColor_[3]);
}
