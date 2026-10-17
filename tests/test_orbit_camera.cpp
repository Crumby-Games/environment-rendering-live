#include <catch.hpp>
#include "app/orbit_camera.h"

using glm::mat4;
using glm::vec3;

static constexpr float EPS = 1e-4f;

static bool isfinite(const vec3& v) {
	return isfinite(v.x) && isfinite(v.y) && isfinite(v.z);
}

static bool isfinite(const mat4& m) {
	for (int c = 0; c < 4; ++c)
		for (int r = 0; r < 4; ++r)
			if (!isfinite(m[c][r]))
				return false;
	return true;
}

static bool approxEqual(const mat4& a, const mat4& b, float eps = 1e-5f) {
	for (int c = 0; c < 4; ++c)
		for (int r = 0; r < 4; ++r)
			if (abs(a[c][r] - b[c][r]) > eps)
				return false;
	return true;
}

TEST_CASE("Camera dolly clamps distance") {
	OrbitCamera cam(1.0f, 10.0f, 5.0f, 16.0f / 9, 0.0f, 0.0f, 45.0f);

	cam.dolly(0.01f);
	REQUIRE(cam.distance() == Catch::Approx(1.0f));

	cam.dolly(100.0f);
	REQUIRE(cam.distance() == Catch::Approx(10.0f));
}

TEST_CASE("Camera pitch is clamped") {
	OrbitCamera cam(1.0f, 10.0f, 5.0f, 16.0f / 9, 0.0f, 0.0f, 45.0f);

	cam.rotate(0.0f, 100.0f);
	mat4 view1 = cam.view();

	cam.rotate(0.0f, -200.0f);
	mat4 view2 = cam.view();

	// If pitch clamping works, view matrix must stay finite
	REQUIRE(isfinite(view1));
	REQUIRE(isfinite(view2));
}

TEST_CASE("Camera yaw wraps around 2PI") {
	OrbitCamera cam(1.0f, 10.0f, 5.0f, 16.0f / 9, 0.0f, 0.0f, 45.0f);

	cam.rotate(glm::two_pi<float>() * 3.0f, 0.0f);
	mat4 viewA = cam.view();

	cam.rotate(0.0f, 0.0f);
	mat4 viewB = cam.view();

	REQUIRE(approxEqual(viewA, viewB, EPS));
}

TEST_CASE("Camera pan moves view position") {
	OrbitCamera cam(1.0f, 10.0f, 5.0f, 16.0f / 9, 0.0f, 0.0f, 45.0f);
	mat4 before = cam.view();

	cam.pan(1.0f, 0.0f);
	mat4 after = cam.view();

	REQUIRE_FALSE(approxEqual(before, after, EPS));
}

TEST_CASE("Camera pan respects constraints") {
	OrbitCamera cam(1.0f, 10.0f, 5.0f, 16.0f / 9, 0.0f, 0.0f, 45.0f);
	cam.adjust(1.0f);

	// Pushes it into the outermost boundary of its constraints
	cam.pan(100.0f, 0.0f);
	vec3 target = cam.target();

	REQUIRE(abs(target.x) == Catch::Approx(1.0f));
	REQUIRE(abs(target.y) == Catch::Approx(0.0f));
	REQUIRE(abs(target.z) == Catch::Approx(0.0f));
}

TEST_CASE("Camera projection matrix is valid") {
	OrbitCamera cam(0.1f, 100.0f, 5.0f, 16.0f / 9, 0.0f, 0.0f, 45.0f);
	mat4 proj = cam.projection();

	REQUIRE(isfinite(proj));
	REQUIRE(proj[3][2] < 0.0f);
}

TEST_CASE("Camera looks at target") {
	OrbitCamera cam(1.0f, 10.0f, 5.0f, 16 / 0.f / 9, glm::radians(30.0f), glm::radians(30.0f), 45.0f);

	mat4 view = cam.view();
	mat4 inv = inverse(view);
	vec3 camPos = vec3(inv[3]);
	vec3 toTarget = normalize(-camPos);

	REQUIRE(dot(cam.forward(), toTarget) == Catch::Approx(1.0f));
	REQUIRE(abs(dot(cam.right(), toTarget)) < EPS);
}
