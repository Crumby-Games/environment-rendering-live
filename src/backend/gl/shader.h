#pragma once
#include <glm/glm.hpp>
#include <string>
#include <variant>
#include "resource.h"

// Wrapper for OpenGL shader programs. Also handles GLSL compilation and parsing.
class GLShader : public GLResource<GLShader> {
public:
	// Supported types of uniforms. (Textures are valid, but should be provided by their handle)
	using UniformValue = std::variant<
		bool,
		float,
		int,
		size_t,
		glm::vec2,
		glm::vec3,
		glm::vec4,
		glm::mat2,
		glm::mat3,
		glm::mat4
	>;

	// Compiles and links two shaders into a program
	GLShader(const std::string& vertPath, const std::string& fragPath);
	// Compiles shader directly from file.
	static GLuint compileShader(const std::string& path);
	// Creates program and attaches shaders
	static GLuint createProgram(GLuint vertexShader, GLuint fragmentShader);
	// Set uniform for use in GLSL. Not persistent; discarded when shader changes. Assumes being bound.
	void setUniform(const std::string& name, const UniformValue& value) const;
	// Check whether this shader defines a particular uniform. Does not require being bound.
	// NOTE: even if the GLSL script contains a uniform definition, it will be optimised away during compilation if unused.
	bool hasUniform(const std::string& name) const noexcept;

	inline static constexpr std::string_view Type = "GLShader";
	void release();
	void use() const noexcept;
};
