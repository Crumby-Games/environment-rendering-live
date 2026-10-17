#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include "mesh.h"

// All data required to draw a 3D model to the screen
class Model {
public:
	explicit Model(Mesh&& m) : mesh_(std::move(m)) {
		// Change position of mesh to be centered
		auto& [min, max] = mesh_.bounds();
		position_ = -(min + max) / 2.0f;
	}
	// Calculates transform matrix based on position and rotation. Rotation is applied as Euler angles in this order: yaw, pitch, roll.
	glm::mat4 matrix() const noexcept {
		glm::mat4 transform = glm::yawPitchRoll(rotation_.x, rotation_.y, rotation_.z);
		transform = glm::translate(transform, position_);
		return transform;
	}
	// Rotation (in radians) of mesh around the y axis (up).
	float& yaw() noexcept { return rotation_.x; }
	// Rotation (in radians) of mesh around the y axis (up).
	float yaw() const noexcept { return rotation_.x; }
	// Rotation (in radians) of mesh around the x axis (right).
	float& pitch() noexcept { return rotation_.y; }
	// Rotation (in radians) of mesh around the x axis (right).
	float pitch() const noexcept { return rotation_.y; }
	// Rotation (in radians) of mesh around the z axis (forward).
	float& roll() noexcept { return rotation_.z; }
	// Rotation (in radians) of mesh around the z axis (forward).
	float roll() const noexcept { return rotation_.z; }
	// Position offset of mesh in 3D space.
	glm::vec3& position() noexcept { return position_; }
	// Position offset of mesh in 3D space.
	glm::vec3 position() const noexcept { return position_; }
	// CPU-side mesh data
	const Mesh& mesh() const noexcept { return mesh_; }
	// CPU-side mesh data
	Mesh& mesh() noexcept { return mesh_; }
private:
	Mesh mesh_;
	glm::vec3 position_ = glm::vec3(0.0f);
	glm::vec3 rotation_ = glm::vec3(0.0f);
	float yaw_ = 0.0f;
	float pitch_ = 0.0f;
	float roll_ = 0.0;
};
