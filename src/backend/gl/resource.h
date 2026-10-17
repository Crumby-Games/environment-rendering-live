#pragma once

#include <glad/glad.h>
#include <stdexcept>
#ifdef _DEBUG
#include "dbg.h"
#endif

template<typename Derived>
class GLResource {
public:
	virtual ~GLResource() {
		if (handle_ == 0) return;
		static_cast<Derived*>(this)->release();
		log("deleted");
	}

	GLResource(GLResource& o) = delete;
	GLResource& operator=(GLResource& o) = delete;

	GLResource(GLResource&& o) noexcept {
		handle_ = o.handle_;
		o.handle_ = 0;
	}

	GLResource& operator=(GLResource&& o) noexcept {
		if (handle_ != 0) {
			static_cast<Derived*>(this)->release();
			log("deleted");
		}
		handle_ = o.handle_;
		o.handle_ = 0;
		return *this;
	}

	GLuint handle() const { return handle_; }
protected:
	GLResource() {}
	GLuint handle_ = 0;
	void log(const char* action) const {
#ifdef _DEBUG
		if constexpr (!LoggingResource<Derived>)
			return;
		std::cout
			<< "[GLResource] " << Derived::Type
			<< " @ " << this
			<< " (" << handle_ << ")"
			<< " | " << action << std::endl;
#endif
	}
};

#undef LOG_FILTER
