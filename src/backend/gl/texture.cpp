#include "texture.h"
#include <cmath>

GLTexture::Preset GLTexture::Preset::CubeMap() {
	return {
		GL_TEXTURE_CUBE_MAP,
		GL_RGB16F,
		GL_RGB,
		GL_FLOAT,
		{
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR },
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR },
			{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE },
			{ GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE }
		}
	};
}

GLTexture::Preset GLTexture::Preset::Noise() {
	return {
		GL_TEXTURE_2D,
		GL_RGB16F,
		GL_RGB,
		GL_FLOAT,
		{
			{ GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{ GL_TEXTURE_MAG_FILTER, GL_NEAREST},
			{ GL_TEXTURE_WRAP_S, GL_REPEAT},
			{ GL_TEXTURE_WRAP_T, GL_REPEAT}
		}
	};
}

GLTexture::Preset GLTexture::Preset::Image() {
	return {
		GL_TEXTURE_2D,
		GL_RGB16F,
		GL_RGB,
		GL_FLOAT,
		{
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR},
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR},
			{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
			{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
		}
	};
}

GLTexture::Preset GLTexture::Preset::MipImage() {
	return {
		GL_TEXTURE_2D,
		GL_RGB16F,
		GL_RGB,
		GL_FLOAT,
		{
			{ GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR },
			{ GL_TEXTURE_MAG_FILTER, GL_LINEAR},
			{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
			{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
		}
	};
}

GLTexture::Preset GLTexture::Preset::ColorAttachment() {
	return {
		GL_TEXTURE_2D,
		GL_RGBA16F,
		GL_RGBA,
		GL_FLOAT,
		{
			{ GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{ GL_TEXTURE_MAG_FILTER, GL_NEAREST},
			{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
			{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
		}
	};
}

GLTexture::Preset GLTexture::Preset::DepthAttachment() {
	return {
		GL_TEXTURE_2D,
		GL_DEPTH_COMPONENT24,
		GL_DEPTH_COMPONENT,
		GL_UNSIGNED_INT,
		{
			{ GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{ GL_TEXTURE_MAG_FILTER, GL_NEAREST},
			{ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
			{ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE}
		}
	};
}

GLTexture::GLTexture(Preset preset) : GLResource(),
	target_(preset.target),
	internalFormat_(preset.internalFormat),
	format_(preset.format),
	type_(preset.dataType)
{
	glGenTextures(1, &handle_);
	glBindTexture(target_, handle_);

	for (auto& [k, v] : preset.parami) {
		glTexParameteri(target_, k, v);
	}
	log("created");
}

void GLTexture::copy(const GLTexture& from) {
	if (internalFormat_ != from.internalFormat_) throw std::runtime_error("Could not copy Texture: GL internal formats do not match");
	if (format_ != from.format_) throw std::runtime_error("Could not copy Texture: GL formats do not match");
	if (type_ != from.type_) throw std::runtime_error("Could not copy Texture: GL types do not match");

	allocate(from.width_, from.height_);

	glCopyImageSubData(
		from.handle_, from.target_, 0, 0, 0, 0,
		handle_, target_, 0, 0, 0, 0,
		width_, height_, 1
	);
	if (levels_ > 1) {
		glGenerateMipmap(target_);
	}
}

void GLTexture::release() { glDeleteTextures(1, &handle_); }

void GLTexture::bind(int unit) const noexcept {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(target_, handle_);
}

void GLTexture::setMipLevels(int levels) {
	levels_ = levels;
	glBindTexture(target_, handle_);
	glTexParameteri(target_, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target_, GL_TEXTURE_MAX_LEVEL, levels_ - 1);
}

int GLTexture::calcMipLevels(int width, int height) noexcept {
	return 1 + (int)std::floor(std::log2(std::max(width, height)));
}

void GLTexture::allocate(int width, int height) {
	if (target_ == GL_TEXTURE_CUBE_MAP)
		throw std::runtime_error("Failed to allocate space for texture: Texture target GL_TEXTURE_CUBE_MAP is not supported. Use allocateCubeFaces() instead.");
	glBindTexture(target_, handle_);
	width_ = width; height_ = height;
	glTexImage2D(target_, 0, internalFormat_, width, height, 0, format_, type_, nullptr);
	if (levels_ > 1) {
		glGenerateMipmap(target_);
	}
}

void GLTexture::clear() const noexcept {
	glBindTexture(target_, handle_);
	glClearTexImage(handle_, 0, format_, type_, nullptr);
	if (levels_ > 1) glGenerateMipmap(target_);
}

void GLTexture::allocateCubeFaces(int size) {
	if (target_ != GL_TEXTURE_CUBE_MAP)
		throw std::runtime_error("Failed to allocate space for cube faces: Texture target is not GL_TEXTURE_CUBE_MAP.");
	glBindTexture(target_, handle_);
	width_ = size; height_ = size;
	for (int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat_, size, size, 0, format_, type_, nullptr);
	}
}

void GLTexture::set(int width, int height, const void* data) {
	if (!data) throw std::runtime_error("Failed to set data for texture: Data is null");
	glBindTexture(target_, handle_);
	switch (target_) {
	case GL_TEXTURE_2D:
		width_ = width; height_ = height;
		glTexImage2D(target_, 0, internalFormat_, width, height, 0, format_, type_, data);
		if (levels_ > 1) glGenerateMipmap(target_);
		return;
	default:
		throw std::runtime_error("Failed to set data: Unsupported texture target.");
	}
}
