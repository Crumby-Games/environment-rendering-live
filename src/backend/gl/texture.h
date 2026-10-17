#pragma once
#include <vector>
#include "resource.h"

// Abstraction of OpenGL boilerplate. Cleanly handles GPU resource lifetime
class GLTexture : public GLResource<GLTexture> {
public:
	// To specify the location of the texture in a shader
	struct Binding {
		const GLTexture& target;
		GLuint unit;

		void activate() const { target.bind(unit); }
	};

	// Parameters and format data to use when constructing a texture
	// @pararm target: the texture "slot"
	// @param internalFormat: how the GPU stores the texture
	// @param format: how the texture is provided
	// @param dataType: type of each component in the data
	struct Preset {
		GLenum target;
		GLenum internalFormat;
		GLenum format;
		GLenum dataType;
		std::vector<std::pair<GLenum, GLenum>> parami; // Integer parameters

		/* Common use cases */

		static Preset CubeMap();
		static Preset Noise();
		static Preset Image();
		static Preset MipImage();
		static Preset ColorAttachment();
		static Preset DepthAttachment();
	};

	GLTexture(Preset preset);
	inline static constexpr std::string_view Type = "GLTexture";
	void release();

	// Set the allocated size of the texture in memory without uploading data
	void allocate(int width, int height);
	// Sets the allocated size of each cube face (assuming a GL_TEXTURE_CUBE_MAP target)
	void allocateCubeFaces(int size);
	// Updates mip state and set necessary parameters for mip-enabled textures
	void setMipLevels(int levels);
	// Returns the maximumum possible number of mip levels for a texture of these dimensions
	static int calcMipLevels(int width, int height) noexcept;
	// Upload data to texture
	void set(int width, int height, const void* data);
	// Explicit syntax for copying GPU texture data. because the method is slow and should not be done by accident, especially not in real-time application.
	void copy(const GLTexture& other);
	// Activate and bind texture at unit
	void bind(int unit) const noexcept;
	// Delete all data in texture
	void clear() const noexcept;

	GLenum glTarget() const noexcept { return target_; }
	GLenum glInternalFormat() const noexcept { return internalFormat_; }
	GLenum glFormat() const noexcept  { return format_; }
	GLenum glType() const noexcept  { return type_; }
	int mipLevels() const noexcept  { return levels_; }
	int width() const noexcept { return width_; }
	int height() const noexcept { return height_; }
private:
	GLenum target_, internalFormat_, format_, type_;
	int levels_ = 1;
	int width_ = 0, height_ = 0;
};
