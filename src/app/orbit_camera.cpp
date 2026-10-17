#include "orbit_camera.h"
#include <iostream>

using glm::mat4;
using glm::vec3;

void OrbitCamera::adjust(float boundsRadius) {
	float distanceFactor = (distance_ - minDist_) / (maxDist_ - minDist_); // Normalised distance between minimum distance and maximum distance

	// Set new constraints and values while keeping ratios in tact
	minDist_ = boundsRadius;
	maxDist_ = boundsRadius * 20.0f;
	distance_ = minDist_ + distanceFactor * (maxDist_ - minDist_);
	target_ = boundsRadius_ ? target_ * (boundsRadius / boundsRadius_) : vec3(0.0);
	boundsRadius_ = boundsRadius;
}

mat4 OrbitCamera::view() const noexcept {
	vec3 position = target_ - forward() * distance_;
	return glm::lookAt(position, target_, vec3(0, 1, 0));
}

void OrbitCamera::rotate(float yaw, float pitch) noexcept {
	yaw_ += yaw;
	yaw_ = glm::mod(yaw_, glm::two_pi<float>());

	pitch_ += pitch;
	pitch_ = glm::clamp(pitch_, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);
}

void OrbitCamera::pan(float x, float y) noexcept {
	float projectedHeight = calculateFrustumHeight(distance_);
	vec3 panOffset = -(
		x * right() * projectedHeight * aspect()
		+ y * up() * projectedHeight
		);
	target_ += panOffset;

	if (boundsRadius_ == 0) return;
	{
		vec3 direction = glm::normalize(target_);
		float distance = glm::length(target_);
		target_ = distance > boundsRadius_ ? direction * boundsRadius_ : target_;
	}
}

void OrbitCamera::dolly(float factor) noexcept {
	distance_ *= factor;
	distance_ = glm::clamp(distance_, minDist_, maxDist_);
}

vec3 OrbitCamera::forward() const noexcept {
	return -vec3{
		cos(pitch_) * sin(yaw_),
		sin(pitch_),
		cos(pitch_) * cos(yaw_)
	};
}

vec3 OrbitCamera::right() const noexcept {
	return glm::normalize(glm::cross(vec3(0, 1, 0), forward()));
}

vec3 OrbitCamera::up() const noexcept {
	return glm::normalize(glm::cross(forward(), right()));
}
