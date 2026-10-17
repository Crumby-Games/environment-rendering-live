#pragma once
#include <unordered_map>
#include <vector>
#include "backend/gl/shader.h"
#include "backend/gl/attachment.h"

// Acts as a persistent layer to store shader uniforms
class ShaderMaterial {
public:
	ShaderMaterial(const std::string& fragShader, const std::string& vertShader);
	// Sets material property internally
	void set(const std::string& name, GLShader::UniformValue value);
	// Sets texture as a material property, which is automatically activated and bound to read when this material is used.
	void bind(const std::string& name, const GLTexture& readTarget);
	// Sets attachment (one or two grouped textures for pingponging) as a material property.
	// Stores reference of target so that the front texture is automatically activated and bound to read when this material is used.
	void bind(const std::string& name, const GLAttachment& readTarget);
	// Bind associated shader, and apply material properties and readable textures.
	void use() const;
	// Return reference of value associated with name
	GLShader::UniformValue* get(const std::string& name) noexcept { return uniforms_.contains(name) ? &uniforms_[name] : nullptr; }
	// Return whether this material has had a specific uniform set before
	// NOTE: to check whether the shader itself has an active uniform, use shader().hasUniform()
	bool has(const std::string& name) const noexcept { return uniforms_.contains(name); }
	// The underlying shader that this material uses. Owned by the material.
	const GLShader& shader() const noexcept { return shader_; }
private:
	std::unordered_map<std::string, GLShader::UniformValue> uniforms_;
	std::vector<GLAttachment::Binding> dynamicBindings_;
	std::vector<GLTexture::Binding> staticBindings_;
	GLShader shader_;
};
