#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

// Abstract class for storing and modifying data relating to the projection and view matrices
class Camera {
public:
	// Calculate and return projection matrix
	glm::mat4 projection() const noexcept {
		return glm::perspective(glm::radians(fov_), aspect_, near(), far());
	}
	// Calculate and return view matrix
	virtual glm::mat4 view() const noexcept = 0;
	// The closest distance from camera that is included in projection
	virtual float near() const noexcept = 0;
	// The furthest distance from camera that is included in projection
	virtual float far() const noexcept = 0;

	// Aspect ratio (w/h)
	float& aspect() noexcept { return aspect_; }
	// Aspect ratio (w/h)
	float aspect() const noexcept { return aspect_; }

	// Vertical field of view (in degrees)
	float& fov() noexcept { return fov_; }
	// Vertical field of view (in degrees)
	float fov() const noexcept { return fov_; }
protected:
	Camera(float aspect, float fov = 60.0) : aspect_(aspect), fov_(fov) {
	}
private:
	float fov_;
	float aspect_;
};