#pragma once
#include <vector>
#include <unordered_map>
#include "attachment.h"
#include "resource.h"

// Abstract class that acts as an abstraction of framebuffer usage
class GLFramebuffer : public GLResource<GLFramebuffer> {
public:
	enum class AttachSlot : uint8_t {
		COLOR0,
		COLOR1,
		COLOR2,
		COLOR3,
		DEPTH,
	};

	// Constructs an FBO with attachments that are allocated that much space
	GLFramebuffer(int width, int height);
	inline static constexpr std::string_view Type = "GLFramebuffer";
	void release();

	// Creates and binds attachment using the relevant Image::Preset. Owns the attachment textures.
	// (Part of the fluent API; can be used in a chain of methods to configure a framebuffer)
	GLFramebuffer& generateDefaultAttachment(AttachSlot slot, bool pingpong);
	// Creates a dynamic attachment which has a texture that is not owned by the framebuffer. (Does not support pingponging)
	// Used when the framebuffer should not own an attachment, such as precomputation passes.
	// (Part of the fluent API; can be used in a chain of methods to configure a pass)
	GLFramebuffer& bindAttachment(AttachSlot slot, GLTexture& texture, int mipLevel = 0);

	// Clears buffers according to clearColor_ and buffermask()
	void clear() noexcept;
	// Prepares GL state. Called before rendering the first time in a sequence that this framebuffer is the target
	void prepare() noexcept;
	// Called every time after rendering to it
	void update();
	// Allocates new size to all attachment textures
	void resize(const int width, const int height);
	// Returnsattachment at given slot.
	GLAttachment* findAttachment(AttachSlot slot) noexcept { return attachments_.contains(slot) ? &attachments_.at(slot) : nullptr; }
	// Force removal of an attachment without updating GL state. External attachment textures (from bindAttachment()) will not be released.
	void destroyAttachment(AttachSlot slot);
	// Set the framebuffer texture to whichever attachment is bound or generated at the slot.
	// @param targetOverride: Use when the framebuffer target should be different than the attachment's texture target, such as when the attachment is a GL_TEXTURE_CUBE_MAP. updateAttachedCubeFace() exists as shorthand for this use case.
	// @param mipLevel: Use when the framebuffer should manually override a non-zero mip of the texture, such as when precomputing a custom mipmap for a texture.
	void updateAttachment(AttachSlot slot, GLenum targetOverride = 0, int mipLevel = 0) const;
	// Calls updateAttachment with a GL_TEXTURE_CUBE_[face] target that corresponds to the face index.
	void updateAttachedCubeFace(AttachSlot slot, int face, int mipLevel = 0) const;
	// Binds the default framebuffer with the default draw buffers
	static void PrepareDefault() noexcept;
	// Clears the default framebuffer of both color and depth.
	static void ClearDefault() noexcept;
	int width() const noexcept { return width_; }
	int height() const noexcept { return height_; }
private:
	void addDrawBuffer(AttachSlot slot) noexcept;
	GLbitfield buffermask() const noexcept; // Selects only buffers it has attachments for
	std::unordered_map<AttachSlot, GLAttachment> attachments_;
	std::vector<GLenum> drawBuffers_;
	int width_, height_;
};
