#pragma once
#include <vector>
#include "resource.h"

class GLBuffer : public GLResource<GLBuffer> {
public:
	GLBuffer(const void* data, GLsizeiptr size);
	template<typename DataType>
	GLBuffer(const std::vector<DataType>& data) :
		GLBuffer(data.data(), data.size() * sizeof(DataType)) {
	}

	inline static constexpr std::string_view Type = "GLBuffer";
	void release();
	void bind() const noexcept;
};

class GLVertexArray : public GLResource<GLVertexArray> {
public:
	GLVertexArray();
	void setAttrib(GLuint index, const GLBuffer& buffer, GLuint componentsPerVertex = 3); // Assumes this is bound already

	inline static constexpr std::string_view Type = "GLVertexArray";
	void release();
	void bind() const noexcept;
};
