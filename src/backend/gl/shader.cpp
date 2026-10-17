#include "shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <sstream>

GLShader::GLShader(const std::string& vertPath, const std::string& fragPath) : GLResource() {
	handle_ = createProgram(compileShader(vertPath), compileShader(fragPath));
	log("created");
}

void GLShader::release() { glDeleteProgram(handle_); }

void GLShader::use() const noexcept { glUseProgram(handle_); }

GLuint GLShader::compileShader(const std::string& path) {
	bool isRelative = std::filesystem::path(path).is_relative();
	const std::string& filepath = isRelative ? SHADER_DIR + path : path;
	// Determine type of shader based on its extension
	GLenum shaderType;
	{
		const std::string ext = filepath.substr(filepath.find_last_of('.') + 1);
		if (ext == "vert") shaderType = GL_VERTEX_SHADER;
		else if (ext == "frag") shaderType = GL_FRAGMENT_SHADER;
		else throw std::runtime_error("GLShader extension unsupported: " + filepath);
	}

	// Get content of shader as a string
	std::string shaderText;
	{
		std::ifstream file(filepath);
		if (!file) throw std::runtime_error("Cannot open shader file: " + filepath);
		std::stringstream ss;
		ss << file.rdbuf();
		shaderText = ss.str();
	}

	// Compiler shader
	GLuint shader = glCreateShader(shaderType);
	{
		const GLchar* source = shaderText.c_str();
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);

	}

	// Determine success
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		// Return error
		char log[512];
		glGetShaderInfoLog(shader, 512, NULL, log);
		throw std::runtime_error("GLShader compilation failed: " + filepath + '\n' + log);
	}

	// Return handle
	return shader;
}

GLuint GLShader::createProgram(GLuint vertexShader, GLuint fragmentShader) {
	// Create program and take shaders, then link them to program
	GLuint program = glCreateProgram();
	{
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);
	}

	// Determine success
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success) {
		// Return error
		char log[512];
		glGetProgramInfoLog(program, 512, NULL, log);
		throw std::runtime_error(std::string("GLShader link failed:\n") + log);
	}

	// Remove shader source from memory
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);

	if (!success) {
		char log[512];
		glGetProgramInfoLog(program, 512, NULL, log);
		throw std::runtime_error(std::string("Program validation failed:\n") + log);
	}

	return program;
}

bool GLShader::hasUniform(const std::string& name) const noexcept {
	return glGetUniformLocation(handle_, name.c_str()) != -1;
}

void GLShader::setUniform(const std::string& name, const UniformValue& value) const {
	GLint location = glGetUniformLocation(handle_, name.c_str());
	if (location == -1) throw std::runtime_error("Shader has no uniform " + name);

	// Determine type of value at runtime and use different GL call based on it
	auto glCall = [location](auto&& v) {
		// Type of value, stripped of const and &.
		using T = std::decay_t<decltype(v)>;

		if constexpr (std::is_same_v<T, float>) {
			glUniform1f(location, v);
		}
		else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, size_t> || std::is_same_v<T, bool>) {
			glUniform1i(location, v);
		}
		else if constexpr (std::is_same_v<T, glm::vec2>) {
			glUniform2fv(location, 1, glm::value_ptr(v));
		}
		else if constexpr (std::is_same_v<T, glm::vec3>) {
			glUniform3fv(location, 1, glm::value_ptr(v));
		}
		else if constexpr (std::is_same_v<T, glm::vec4>) {
			glUniform4fv(location, 1, glm::value_ptr(v));
		}
		else if constexpr (std::is_same_v<T, glm::mat2>) {
			glUniformMatrix2fv(location, 1, GL_FALSE, value_ptr(v));
		}
		else if constexpr (std::is_same_v<T, glm::mat3>) {
			glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(v));
		}
		else if constexpr (std::is_same_v<T, glm::mat4>) {
			glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(v));
		}
		};

	// Ensures all variant types are checked
	std::visit(glCall, value);
}
