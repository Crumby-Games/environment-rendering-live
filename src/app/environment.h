#pragma once

#include <string>
#include <glm/glm.hpp>
#include "engine/image.h"
#include "backend/gl/texture.h"
#include "backend/gl/shader.h"
#include "backend/gl/renderer.h"
#include "backend/gl/framebuffer.h"

class Environment {
public:
	Environment();
	void load(const Image& image);
	void load(const std::string& filepath);
	void reset();
	void recompute(bool irradiance = true, bool filter = true);

	void applyRoughness(float value);
	void applyLod(float lod);
	void applyResolution(int textureSize);

	float roughness() const noexcept { return roughness_; }
	int lod() const noexcept { return lod_; }
	int resolution() const noexcept { return size_; }
	GLTexture& noise() noexcept { return noise_; }
	const std::vector<glm::vec3>& kernel() const noexcept { return ssaoKernel_; }
	GLTexture& map() noexcept { return map_; }
	const GLTexture& map() const noexcept { return map_; }
	GLTexture& irradianceMap() noexcept { return irradianceMap_; }
	const GLTexture& irradianceMap() const noexcept { return irradianceMap_; }
	GLTexture& filteredMap() noexcept { return filteredMap_; }
	const GLTexture& filteredMap() const noexcept { return filteredMap_; }
private:
	void generateNoiseTexture(int size);
	void generateKernel(int size);
	void performIrradiancePass(GLFramebuffer& fbo);
	void performFilterPass(GLFramebuffer& fbo);
	GLRenderer renderer_;
	GLShader irradianceShader_ = GLShader("core/fullscreen.vert", "app/irradiance.frag");
	GLShader filterShader_ = GLShader("core/fullscreen.vert", "app/filter.frag");
	GLTexture noise_ = GLTexture::Preset::Noise();
	std::vector<glm::vec3> ssaoKernel_;
	GLTexture map_ = GLTexture::Preset::MipImage();
	GLTexture irradianceMap_ = GLTexture::Preset::Image();
	GLTexture filteredMap_ = GLTexture::Preset::Image();
	int maxMipLevels_ = 1;
	float roughness_ = 1.0f;
	float lod_ = 0.0f;
	int size_ = 512;
};
