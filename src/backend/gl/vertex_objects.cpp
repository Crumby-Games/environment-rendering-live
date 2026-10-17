#include "vertex_objects.h"
#include <glad/glad.h>

GLBuffer::GLBuffer(const void* data, GLsizeiptr size) : GLResource() {
	glGenBuffers(1, &handle_);
	bind();
	glBufferData(
		GL_ARRAY_BUFFER,
		size,
		data,
		GL_STATIC_DRAW
	);
	log("created");
}

void GLBuffer::release() { glDeleteBuffers(1, &handle_); }
void GLBuffer::bind() const noexcept { glBindBuffer(GL_ARRAY_BUFFER, handle_); }

GLVertexArray::GLVertexArray() : GLResource() {
	glGenVertexArrays(1, &handle_);
	log("created");
}

void GLVertexArray::release() { glDeleteVertexArrays(1, &handle_); }
void GLVertexArray::bind() const noexcept { glBindVertexArray(handle_); }

void GLVertexArray::setAttrib(GLuint index, const GLBuffer& buffer, GLuint componentsPerVertex) {
	buffer.bind();
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
}
