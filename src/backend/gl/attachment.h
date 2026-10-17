#pragma once
#include <optional>
#include "texture.h"

// Built on top of Texture class to be used in the pipeline with framebuffers directly
class GLAttachment {
public:
	// To specify the location of the target attachment in a shader
	struct Binding {
		const GLAttachment& target;
		GLuint unit;

		void activate() const { target.read().bind(unit); }
	};

	// Creates either one or two textures, depending on whether pingponging is needed
	GLAttachment(GLTexture::Preset preset, bool pingpong) {
		write_ = preset;
		if (pingpong) read_ = preset;
	}

	// Construct GLAttachment as a wrapper for an external texture
	GLAttachment(GLTexture& texture)
	{
		external_ = &texture;
	}

	// Switches textures if there are multiple
	void swap() { if (read_ && write_) std::swap(write_, read_); }
	// Returns the front texture. If constructed without an internal front texture, return the external texture.
	GLTexture& write() noexcept { return write_ ? *write_ : *external_; }
	const GLTexture& write() const noexcept { return write_ ? *write_ : *external_; }

	// Returns the back texture if there are multiple, otherwise returns the front texture (which may return the external texture)
	GLTexture& read() noexcept { return read_ ? *read_ : write(); }
	const GLTexture& read() const noexcept { return read_ ? *read_ : write(); }

	// Specify size of textures
	void allocate(int width, int height) {
		if (external_) external_->allocate(width, height);
		else if (write_) {
			write_->allocate(width, height);
			if (read_) read_->allocate(width, height);
		}
	}
	void clear() {
		if (external_) external_->clear();
		else if (write_) {
			write_->clear();
			if (read_) read_->clear();
		}
	}


	// Whether this is constructed internally and has two textures
	bool separated() const noexcept { return read_ && write_; }

	// Disable copy assignment and copy construction because this class uses GLResources
	GLAttachment(GLAttachment&) = delete;
	GLAttachment& operator=(GLAttachment&) = delete;
	GLAttachment(GLAttachment&&) noexcept = default;
	GLAttachment& operator=(GLAttachment&&) noexcept = default;
private:
	GLTexture* external_ = nullptr;
	std::optional<GLTexture> write_;
	std::optional<GLTexture> read_;
};
