#include "pipeline.h"
#include <unordered_set>
#include <format>

Pipeline::Pipeline(int width, int height) : width_(width), height_(height) {}

void Pipeline::execute() const {
	const GLRenderer* prevRenderer = nullptr;
	std::unordered_set<GLFramebuffer*> clearedFbos;
	for (auto& passName : passOrder_) {
		const Pass& pass = passes_.at(passName);
		bool defaultFB = pass.target() == nullptr;

		if (defaultFB) GLFramebuffer::PrepareDefault();
		else pass.target()->prepare();

		// Only clear an FBO if it hasn't been cleared already this frame
		if (!clearedFbos.contains(pass.target())) {
			if (defaultFB) GLFramebuffer::ClearDefault();
			else pass.target()->clear();

			clearedFbos.insert(pass.target());
		}

		if (&pass.renderer() != prevRenderer) {
			pass.renderer().prepare();
			prevRenderer = &pass.renderer();
		}

		pass.execute();
	}
}

GLFramebuffer& Pipeline::createFBO(const std::string& name) {
	if (fbos_.contains(name)) throw std::runtime_error(std::format("Framebuffer \"{}\" already exists.", name));
	fbos_.try_emplace(name, width_, height_);
	if (!fbos_.contains(name)) throw std::runtime_error(std::format("Failed to create Framebuffer \"{}\" with dimensions {}x{}", name, width_, height_));
	return fbos_.at(name);
}

void Pipeline::resizeViewport(int width, int height) {
	width_ = width; height_ = height;
	for (auto& [_, fbo] : fbos_) fbo.resize(width, height);
	for (auto& [_, renderer] : renderers_) renderer.viewport() = { 0, 0, width, height };
}

GLRenderer& Pipeline::createRenderer(const std::string& name, const std::string& vertShader, bool depthTesting) {
	if (renderers_.contains(name)) throw std::runtime_error(std::format("Renderer \"{}\" already exists.", name));
	renderers_.try_emplace(name, width_, height_);
	if (!renderers_.contains(name)) throw std::runtime_error(std::format("Failed to create Renderer \"{}\" with dimensions {}x{}", name, width_, height_));
	GLRenderer* renderer = findRenderer(name);
	renderer->depthTesting() = depthTesting;
	vertShaders_.try_emplace(name, vertShader);
	return *renderer;

}

void Pipeline::deleteRenderer(const std::string& name) {
	if (!renderers_.contains(name)) throw std::runtime_error("Renderer \"" + name + "\" does not exist to delete.");
	renderers_.erase(name);
}

Pass& Pipeline::createPass(const std::string& name, const std::string& renderer, const std::string& fragShader) {
	if (passes_.contains(name)) throw std::runtime_error(std::format("Pass \"{}\" already exists.", name));
	if (!renderers_.contains(renderer)) throw std::runtime_error(std::format("Renderer \"{}\" does not exist for use in Pass \"{}\".", renderer, name));
	passes_.emplace(name, Pass(*this, renderer, fragShader, vertShaders_.at(renderer)));
	if (!passes_.contains(name)) throw std::runtime_error(std::format(
		R"(
Failed to create Pass \"{}\".
\tRenderer: \"{}\"
\tFragment Shader: \"{}\"
\tVertex Shader: \"{}\"
		)", name, renderer, fragShader, vertShaders_.at(renderer)));
	Pass& pass = passes_.at(name);
	passOrder_.push_back(name);
	return pass;
}

Pass& Pipeline::replacePass(const std::string& toReplace, const std::string& name, const std::string& renderer, const std::string& fragShader) {
	if (!passes_.contains(toReplace)) throw std::runtime_error(std::format("Cannot replace pass: \"{}\" does not exist", toReplace));
	if (!renderers_.contains(renderer)) throw std::runtime_error(std::format("Renderer \"{}\" does not exist for use in Pass \"{}\".", renderer, name));
	Pass pass(*this, renderer, fragShader, vertShaders_.at(renderer));
	// When keys are the same, the pass order doesn't need to be changed.
	if (toReplace == name) {
		passes_.erase(name);
		passes_.emplace(name, std::move(pass));
		return passes_.at(name);
	}

	if (!passes_.contains(toReplace)) throw std::runtime_error(std::format("Cannot replace pass: \"{}\" already exists", name));

	for (auto& passName : passOrder_) {
		if (passName == toReplace) {
			passes_.erase(toReplace);
			passes_.emplace(name, std::move(pass));
			passName = name;
			return passes_.at(name);
		}
	}
	throw std::runtime_error(std::format("Could not replace pass: \"{}\" does not appear in the runtime pass sequence", toReplace));
}

Pass& Pipeline::createCopyPass(const std::string& readBuffer) {
	return createPass("copy" + std::to_string(passes_.size()), "fullscreen", "core/copy.frag")
		.read("uColor", readBuffer, GLFramebuffer::AttachSlot::COLOR0);
}

Pass& Pipeline::createCompositePass(const std::string& backBuffer, const std::string& frontBuffer) {
	return createPass("composite" + std::to_string(passes_.size()), "fullscreen", "core/composite.frag")
		.read("uFrontColor", frontBuffer, GLFramebuffer::AttachSlot::COLOR0)
		.read("uBackColor", backBuffer, GLFramebuffer::AttachSlot::COLOR0);
}

Pass& Pipeline::createFillPass(glm::vec3 colour) {
	return createPass("fill" + std::to_string(passes_.size()), "fullscreen", "core/fill.frag")
		.set("uFillColor", colour);
}
