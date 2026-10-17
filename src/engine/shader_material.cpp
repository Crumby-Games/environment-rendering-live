#include "shader_material.h"
#include <format>

ShaderMaterial::ShaderMaterial(const std::string& fragShader, const std::string& vertShader) : shader_(fragShader, vertShader) {};

void ShaderMaterial::set(const std::string& name, GLShader::UniformValue value) {
	if (!shader_.hasUniform(name)) throw std::runtime_error(std::format("Shader has no uniform {}", name));
	uniforms_[name] = value;
}

void ShaderMaterial::bind(const std::string& name, const GLTexture& readTarget) {
	GLuint unit = dynamicBindings_.size() + staticBindings_.size();
	set(name, unit);
	staticBindings_.emplace_back(readTarget, unit);
}

void ShaderMaterial::bind(const std::string& name, const GLAttachment& readTarget) {
	GLuint unit = dynamicBindings_.size() + staticBindings_.size();
	set(name, unit);
	dynamicBindings_.emplace_back(readTarget, unit);
}

void ShaderMaterial::use() const {
	shader_.use();
	for (const auto& [name, value] : uniforms_) shader_.setUniform(name, value);
	for (const auto& binding : staticBindings_) binding.activate();
	for (const auto& binding : dynamicBindings_) binding.activate();
}
