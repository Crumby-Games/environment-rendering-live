#pragma once
#include "engine/camera.h"

class OrbitCamera : public Camera {
public:
	OrbitCamera(float minDist, float maxDistance, float distance, float aspect, float yaw = 0.0f, float pitch = 0.0f, float fov = 60.0f, glm::vec3 target = glm::vec3(0.0f)) :
		Camera(aspect, fov),
		minDist_(minDist),
		maxDist_(maxDistance),
		distance_(distance),
		yaw_(yaw),
		pitch_(pitch),
		target_(target)
	{
	};

	// Adapts camera distance and constraints to new bounds, keeping ratios intact. This makes it seamless to transition to bigger meshes that are otherwise identical.
	void adjust(float boundsRadius);

	// Rotates camera position around the target by yaw and pitch (in radians).
	// Enforces yaw as being in a 2PI range. Constrains pitch between -PI and PI.
	void rotate(float yaw, float pitch) noexcept;

	// Shifts the target position along camera's local axes, relative to the frustum size.
	// Target's distance to origin is clamped by radius of the bounds
	void pan(float x, float y) noexcept;

	// "Zooms" by multiplying target distance by the given factor.
	// Clamps between minimum and maximum distance.
	void dolly(float factor) noexcept;

	// Calculate and return view matrix
	glm::mat4 view() const noexcept override;
	// Calculate height of the cross-section of the viewing frustum that is the distance away from camera.
	float calculateFrustumHeight(float distance) const { return 2.0f * distance * tan(glm::radians(fov()) / 2.0f); }

	// The furthest distance possible without intersecting bounds at the nearest point
	float near() const noexcept override { return boundsRadius_ ? std::max(0.1f, distance_ - boundsRadius_ * 2.0f) : 0.1f; }
	// The closest distance possible without intersecting bounds at the furthest point
	float far() const noexcept override { return boundsRadius_ ? distance_ + boundsRadius_ * 2.0f : 100.0f; }

	// Local x axis
	glm::vec3 forward() const noexcept;
	// Local z axis
	glm::vec3 right() const noexcept;
	// Local y axis
	glm::vec3 up() const noexcept;
	
	// Origin point of orbit
	glm::vec3 target() const noexcept { return target_; }
	// Distance from target (origin)
	float distance() const noexcept { return distance_; }
	// Minimum distance (enforced by clamping) from target (origin)
	float distanceMin() const noexcept { return minDist_; }
	// Maximum distance (enforced by clamping) from target (origin)
	float distanceMax() const noexcept { return maxDist_; }
	// Rotation (in radians) of camera around the y axis (up). i.e. for looking left/right
	float yaw() const noexcept { return yaw_; }
	// Rotation (in radians) of camera around the local z axis (right). i.e. for looking up/down
	float pitch() const noexcept { return pitch_; }
	// Spherical bound area for constraining panning
	float boundsRadius() const noexcept { return boundsRadius_; }
private:
	float minDist_, maxDist_;
	float distance_;
	float yaw_, pitch_;
	float boundsRadius_ = 0;
	glm::vec3 target_;
};
