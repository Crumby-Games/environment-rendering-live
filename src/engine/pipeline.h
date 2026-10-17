#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "pass.h"
#include "backend/gl/framebuffer.h"
#include "backend/gl/renderer.h"

class Pipeline {
public:
	Pipeline(int width, int height);
	// Iterate through and execute passes, preparing renderers and framebuffers as necessary.
	void execute() const;
	// Returns the last pass in sequence. Returns nullptr if there are no configured passes.
	Pass* lastPassOrNull() noexcept { return passOrder_.empty() ? nullptr : &passes_.at(passOrder_.back()); }
	// Returns the internally registered FBO with the given name. Returns nullptr if it does not exist.
	GLFramebuffer* findFBO(const std::string& name) noexcept { return fbos_.contains(name) ? &fbos_.at(name) : nullptr; }
	// Returns the internally registered pass with the given name. Returns nullptr if it does not exist.
	Pass* findPass(const std::string& name) noexcept { return passes_.contains(name) ? &passes_.at(name) : nullptr; }
	// Returns the internally registered renderer with the given name. Returns nullptr if it does not exist.
	GLRenderer* findRenderer(const std::string& name) noexcept { return renderers_.contains(name) ? &renderers_.at(name) : nullptr; }
	// Delete internal renderer with the given name. Throws error if does not exist.
	void deleteRenderer(const std::string& name);
	// Creates and returns a framebuffer, adding it to an internal map of FBOs.
	// Constructs attachments for it based on the stream, where boolean values symbolise pingponging being enabled.
	// Pingponging is required for FBOs which require both reading and writing in a single pass.
	GLFramebuffer& createFBO(const std::string& name);
	// Creates and returns a renderer, adding it to an internal map of Renderers.
	// Also stores path to associated vertex shader without compiling it. 
	GLRenderer& createRenderer(const std::string& name, const std::string& vertShader, bool depthTesting);
	
	// Creates generic pass; assumes vertex shader associated with Renderer (as the result of createRenderer())
	Pass& createPass(const std::string& name, const std::string& renderer, const std::string& fragShader);
	// Replaces pass at given key with a new, uninitialised generic pass (see CreatePass()). Preserves run-time pass order.
	Pass& replacePass(const std::string& toReplace, const std::string& name, const std::string& renderer, const std::string& fragShader);

	const std::unordered_map<std::string, GLRenderer>& renderers() const noexcept { return renderers_; }
	std::unordered_map<std::string, GLRenderer>& renderers() noexcept { return renderers_; }
	const std::unordered_map<std::string, GLFramebuffer>& framebuffers() const noexcept { return fbos_; }
	std::unordered_map<std::string, GLFramebuffer>& framebuffers() noexcept { return fbos_; }
	const std::unordered_map<std::string, Pass>& passes() const noexcept { return passes_; }
	std::unordered_map<std::string, Pass>& passes() { return passes_; }
	void resizeViewport(int width, int height);

	/* PASS SHORTHANDS */

	// core/copy.frag and built-in fullscreen renderer.
	// reads Color0 from a framebuffer
	// writes to Color0
	Pass& createCopyPass(const std::string& read);

	// core/composite.frag and built-in fullscreen renderer.
	// reads Color0 from two framebuffers, drawing frontbuffer on top of backbuffer.
	// writes to Cololr0
	Pass& createCompositePass(const std::string& backBuffer, const std::string& frontBuffer);

	// core/fill.frag and built-in fullscreen renderer
	// writes solid color to Color0
	Pass& createFillPass(glm::vec3 colour = glm::vec3(0));
private:
	int width_, height_;
	std::unordered_map<std::string, GLFramebuffer> fbos_;
	std::unordered_map<std::string, Pass> passes_;
	std::unordered_map<std::string, GLRenderer> renderers_;
	std::unordered_map<std::string, std::string> vertShaders_;
	std::vector<std::string> passOrder_;
};
