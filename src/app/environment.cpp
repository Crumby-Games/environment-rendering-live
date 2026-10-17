#include "environment.h"
#include <random>
#include <glm/gtc/matrix_transform.hpp>

using enum GLFramebuffer::AttachSlot;

Environment::Environment() : renderer_(size_, size_) {
	renderer_.uploadAttributes(
		{ {
		-1.f, -1.f, 0.f,
		1.f, -1.f, 0.f,
		1.f,  1.f, 0.f,

		-1.f, -1.f, 0.f,
		1.f,  1.f, 0.f,
		-1.f,  1.f, 0.f
	} }
	);

	generateNoiseTexture(4);
	generateKernel(32);
}

void Environment::reset() {
	map_.clear();
	irradianceMap_.clear();
	filteredMap_.clear();
}

void Environment::load(const std::string& filepath) {
	Image img(filepath);
	load(img);
}

void Environment::applyRoughness(float value) {
	value = round(value * 5) / 5;
	if (value == roughness_) return;
	roughness_ = value;
	recompute(false, true);
}

void Environment::applyResolution(int value) {
	float factor = float(size_) / value;
	if (factor > 0.8 && factor < 1.2) return;
	size_ = value;
	recompute();
}

void Environment::applyLod(float value) {
	value = round(value * 2) / 2;
	if (value == lod_) return;
	lod_ = value;
	recompute();
}

void Environment::load(const Image& image) {
	maxMipLevels_ = GLTexture::calcMipLevels(image.width(), image.height());
	map_.setMipLevels(maxMipLevels_);
	map_.allocate(image.width(), image.height());
	map_.set(image.width(), image.height(), image.data().data());
	lod_ = maxMipLevels_ - 4;
	recompute();
}

void Environment::recompute(bool irradiance, bool filter) {
	renderer_.viewport() = { 0, 0, size_ * 2, size_ };
	renderer_.prepare();
	GLFramebuffer fbo(size_ * 2, size_);
	if (irradiance) performIrradiancePass(fbo);
	if (irradiance && filter) fbo.destroyAttachment(COLOR0);
	if (filter) performFilterPass(fbo);
}

void Environment::generateNoiseTexture(int size) {
	std::vector<glm::vec3> noise;
	noise.reserve(size * size);

	std::uniform_real_distribution<float> random(-1.0f, 1.0f);
	std::default_random_engine engine;

	for (int i = 0; i < size * size; i++) {
		noise.emplace_back(random(engine),random(engine),0.0f);
	}

	noise_.set(size, size, noise.data());
}

void Environment::generateKernel(int samples) {
	ssaoKernel_.clear();
	ssaoKernel_.reserve(samples);

	std::uniform_real_distribution<float> random(0.0f, 1.0f);
	std::default_random_engine engine;

	for (int i = 0; i < samples; i++) {
		glm::vec3 sample(
			random(engine) * 2.0f - 1.0f,
			random(engine) * 2.0f - 1.0f,
			random(engine)
		);

		sample = glm::normalize(sample);
		sample *= random(engine);

		// scale bias (important for quality)
		float scale = float(i) / float(samples);
		scale = glm::mix(0.05f, 1.0f, pow(scale,3));

		sample *= scale;

		ssaoKernel_.push_back(sample);
	}
}

void Environment::performIrradiancePass(GLFramebuffer& fbo) {
	fbo.bindAttachment(COLOR0, irradianceMap_);
	fbo.prepare();
	irradianceShader_.use();
	irradianceShader_.setUniform("uEnvironment", 0);
	map_.bind(0);
	irradianceShader_.setUniform("uLod", lod_);
	renderer_.render();
}

void Environment::performFilterPass(GLFramebuffer& fbo) {
	fbo.bindAttachment(COLOR0, filteredMap_);
	fbo.prepare();
	filterShader_.use();
	filterShader_.setUniform("uEnvironment", 0);
	map_.bind(0);
	filterShader_.setUniform("uRoughness", roughness_);
	filterShader_.setUniform("uLod", lod_);
	renderer_.render();
}
