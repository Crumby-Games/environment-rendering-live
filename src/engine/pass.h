#pragma once
#include <string>
#include "backend/gl/renderer.h"
#include "backend/gl/framebuffer.h"
#include "backend/gl/shader.h"
#include "shader_material.h"

class Pipeline;

class Pass {
public:
	Pass(Pipeline& pipeline, const std::string& renderer, const std::string& fragShader, const std::string& vertShader);
	// Specify material property and texture to read.
	// (Part of the fluent API; can be used in a chain of methods to configure a pass)
	Pass& read(const std::string& uniformName, GLTexture& texture);
	// Specify material property to read framebuffer attachment to. Throws error if framebuffer or attachment does not exist.
	// (Part of the fluent API; can be used in a chain of methods to configure a pass)
	Pass& read(const std::string& uniformName, const std::string& buffer, GLFramebuffer::AttachSlot slot);
	// Specify target framebuffer. Throws error if framebuffer does not exist.
	// (Part of the fluent API; can be used in a chain of methods to configure a pass)
	Pass& write(const std::string& buffer);
	// Set material property of name to value
	// (Part of the fluent API; can be used in a chain of methods to configure a pass)
	Pass& set(const std::string& name, const GLShader::UniformValue& value);

	// Perform setup and rendering steps required uniquely for this pass.
	void execute() const;
	// Whether the pass is active. If disabled, this pass skips its execution.
	bool& enabled() noexcept { return enabled_; }
	// Whether the pass is active. If disabled, this pass skips its execution.
	bool enabled() const noexcept { return enabled_; }

	// The unique shader material (persistent shader-related data) associated with this pass.
	ShaderMaterial& material() noexcept { return material_; }
	// The unique shader material (persistent shader-related data) associated with this pass.
	const ShaderMaterial& material() const noexcept { return material_; }
	// A vertex renderer to use when drawing. Does not own.
	GLRenderer& renderer() noexcept { return renderer_; }
	// A vertex renderer to use when drawing. Does not own.
	const GLRenderer& renderer() const noexcept { return renderer_; }
	// The framebuffer to draw to, where nullptr means default. Does not own.
	GLFramebuffer* target() const noexcept { return targetBuffer_; }
private:
	const std::string name_;
	Pipeline& pipeline_;
	GLFramebuffer* targetBuffer_ = nullptr; // By default, a pass will write to the default framebuffer
	GLRenderer& renderer_;
	ShaderMaterial material_;
	bool enabled_ = true;
};
