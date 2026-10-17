#pragma once
#include <glad/glad.h>
#include <iostream>

template<typename T>
inline constexpr bool LoggingResource = false;

#define ENABLE_RESOURCE_LOG(Type)									\
	class Type;														\
	template<> inline constexpr bool LoggingResource<Type> = true;

ENABLE_RESOURCE_LOG(GLFramebuffer);
ENABLE_RESOURCE_LOG(GLVertexArray);

inline const char* GLErrorToString(GLenum error) noexcept
{
	switch (error) {
	case GL_NO_ERROR:					  return "GL_NO_ERROR";
	case GL_INVALID_ENUM:				  return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:				 return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:			 return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:				 return "GL_OUT_OF_MEMORY";
	default:							   return "UNKNOWN_ERROR";
	}
}

inline const char* GLFramebufferStatusToString(GLenum status) noexcept
{
	switch (status)
	{
	case GL_FRAMEBUFFER_COMPLETE: return "GL_FRAMEBUFFER_COMPLETE";
	case GL_FRAMEBUFFER_UNDEFINED: return "GL_FRAMEBUFFER_UNDEFINED";
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
	case GL_FRAMEBUFFER_UNSUPPORTED: return "GL_FRAMEBUFFER_UNSUPPORTED";
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
	default: return "UNKNOWN_FRAMEBUFFER_STATUS";
	}
}

inline void glCheckErrors(const char* file, int line, const char* stmt)
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr
			<< "[OpenGL ERROR] "
			<< GLErrorToString(err)
			<< " | " << file << ":" << line
			<< " | " << stmt
			<< std::endl;

		if (err == GL_INVALID_FRAMEBUFFER_OPERATION)
		{
			GLint fbo = 0;
			glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

			std::cerr
				<< " | Status: "
				<< GLFramebufferStatusToString(status)
				<< std::endl;
		}
	}
}

#ifdef _DEBUG
#define GL_CHECK(stmt) \
		do { \
			while (glGetError() != GL_NO_ERROR); \
			stmt; \
			glCheckErrors(__FILE__, __LINE__, #stmt); \
		} while (0)
#else
#define GL_CHECK(stmt) stmt
#endif
