#include "pass.h"
#include <stdexcept>
#include "backend/gl/shader.h"
#include "engine/pipeline.h"

Pass::Pass(Pipeline& pipeline, const std::string& renderer, const std::string& fragShader, const std::string& vertShader) 
	: pipeline_(pipeline), renderer_(*pipeline.findRenderer(renderer)), material_(fragShader, vertShader) {
}

Pass& Pass::set(const std::string& name, const GLShader::UniformValue& value) {
	material_.set(name, value);
	return *this;
}

Pass& Pass::read(const std::string& uniformName, GLTexture& texture) {
	material_.bind(uniformName, texture);
	return *this;
}

Pass& Pass::read(const std::string& uniformName, const std::string& buffer, GLFramebuffer::AttachSlot slot) {
	GLFramebuffer* fb = pipeline_.findFBO(buffer);
	if (!fb) throw std::runtime_error("Framebuffer \"" + buffer + "\" does not exist to read from.");

	GLAttachment* attachment = fb->findAttachment(slot);
	if (!attachment) throw std::runtime_error("Framebuffer \"" + buffer + "\" does not have expected attachment.");
	material_.bind(uniformName, *attachment);
	return *this;
}

Pass& Pass::write(const std::string& buffer) {
	GLFramebuffer* fb = pipeline_.findFBO(buffer);
	if (!fb) throw std::runtime_error("Framebuffer \"" + buffer + "\" does not exist to write to.");
	targetBuffer_ = fb;
	return *this;
}

void Pass::execute() const {
	if (!enabled_) return;
	material_.use();
	renderer_.render();
	if (targetBuffer_) targetBuffer_->update();
}
