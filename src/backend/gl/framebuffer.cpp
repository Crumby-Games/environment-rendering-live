#include "framebuffer.h"
#include <glad/glad.h>

using enum GLFramebuffer::AttachSlot;

GLFramebuffer::GLFramebuffer(int width, int height) : GLResource(), width_(width), height_(height) {
	glGenFramebuffers(1, &handle_);
	glBindFramebuffer(GL_FRAMEBUFFER, handle_);
	log("created");
}

void GLFramebuffer::release() { glDeleteFramebuffers(1, &handle_); }

void GLFramebuffer::clear() noexcept {
	glClear(buffermask());
}

void GLFramebuffer::PrepareDefault() noexcept {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
}

void GLFramebuffer::ClearDefault() noexcept {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static GLenum glSlot(GLFramebuffer::AttachSlot slot) noexcept {
	switch (slot) {
	case COLOR0: return GL_COLOR_ATTACHMENT0;
	case COLOR1: return GL_COLOR_ATTACHMENT1;
	case COLOR2: return GL_COLOR_ATTACHMENT2;
	case COLOR3: return GL_COLOR_ATTACHMENT3;
	case DEPTH: return GL_DEPTH_ATTACHMENT;
	default: return 0;
	}
}

GLFramebuffer& GLFramebuffer::generateDefaultAttachment(AttachSlot slot, bool pingpong) {
	if (attachments_.contains(slot)) throw std::runtime_error("Cannot generate default attachment for Framebuffer: attachment slot already taken");

	switch (slot) {
	case DEPTH:
		attachments_.try_emplace(slot, GLTexture::Preset::DepthAttachment(), pingpong);
		break;
	case COLOR0:
	case COLOR1:
	case COLOR2:
	case COLOR3:
		attachments_.try_emplace(slot, GLTexture::Preset::ColorAttachment(), pingpong);
		break;
	}

	if (!attachments_.contains(slot)) throw std::runtime_error("Something went wrong in generating attachment for Framebuffer");

	addDrawBuffer(slot);
	attachments_.at(slot).allocate(width_, height_);
	updateAttachment(slot);

	return *this;
}

void GLFramebuffer::prepare() noexcept {
	glBindFramebuffer(GL_FRAMEBUFFER, handle_);
	glDrawBuffers(drawBuffers_.size(), drawBuffers_.data());
}

void GLFramebuffer::update() {
	for (auto& [slot, attachment] : attachments_) {
		if (!attachment.separated()) continue;
		attachment.swap();
		updateAttachment(slot);
	}
}

GLbitfield GLFramebuffer::buffermask() const noexcept {
	GLbitfield f = 0;
	if (attachments_.contains(DEPTH)) {
		f |= GL_DEPTH_BUFFER_BIT;
	}

	if (attachments_.contains(COLOR0) ||
		attachments_.contains(COLOR1) ||
		attachments_.contains(COLOR2) ||
		attachments_.contains(COLOR3)) {
		f |= GL_COLOR_BUFFER_BIT;
	}
	return f;
}

void GLFramebuffer::resize(const int width, const int height) {
	glBindFramebuffer(GL_FRAMEBUFFER, handle_);
	for (auto& [slot, attachment] : attachments_) attachment.allocate(width, height);
}

void GLFramebuffer::updateAttachment(AttachSlot slot, GLenum targetOverride, int mipLevel) const {
	if (!attachments_.contains(slot)) throw std::runtime_error("Cannot update attachment of Framebuffer: attachment does not exist");
	const GLTexture& tex = attachments_.at(slot).write();
	GLenum target = tex.glTarget();
	if (targetOverride) target = targetOverride;
	glFramebufferTexture2D(GL_FRAMEBUFFER, glSlot(slot), target, tex.handle(), mipLevel);
}

void GLFramebuffer::updateAttachedCubeFace(AttachSlot slot, int face, int mipLevel) const {
	updateAttachment(slot, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mipLevel);
}

void GLFramebuffer::destroyAttachment(AttachSlot slot) {
	if (!attachments_.contains(slot)) throw std::runtime_error("Cannot destroy attachment of Framebuffer: attachment does not exist");
	attachments_.erase(slot);
	GLenum glslot = glSlot(slot);
	for (int idx = 0; idx < drawBuffers_.size(); idx++) {
		if (drawBuffers_.at(idx) == glslot) {
			drawBuffers_.erase(drawBuffers_.begin() + idx);
			break;
		}
	}
}

GLFramebuffer& GLFramebuffer::bindAttachment(AttachSlot slot, GLTexture& texture, int mipLevel) {
	if (attachments_.contains(slot)) throw std::runtime_error("Cannot generate default attachment for Framebuffer: attachment slot already taken");
	attachments_.try_emplace(slot, texture);
	addDrawBuffer(slot);
	if (texture.glTarget() == GL_TEXTURE_CUBE_MAP) {
		texture.allocateCubeFaces(width_);
		return *this;
	}
	else {
		texture.allocate(width_, height_);
	}
	updateAttachment(slot, mipLevel);
	return *this;
}

void GLFramebuffer::addDrawBuffer(AttachSlot slot) noexcept {
	switch (slot) {
	case COLOR0:
	case COLOR1:
	case COLOR2:
	case COLOR3:
		drawBuffers_.push_back(glSlot(slot));
		break;
	}
}
