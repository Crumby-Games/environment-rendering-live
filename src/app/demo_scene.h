#pragma once
#include "engine/scene.h"
#include "input.h"
#include "environment.h"

class DemoScene : public Scene {
public:
	DemoScene(const int width, const int height);
	virtual ~DemoScene() override = default;
	void process(float delta) override;
	void pullState(Scene& source) override;
	void onResize() override;
	void connectInput(Input& input) noexcept { input_ = &input; }
	// The currently active user-defined fragment shader. Disabled when N/A. Runs on the fullscreen renderer.
	void setUserShader(const std::string& fragShader);
	// Replaces mesh of model and updates related data
	void changeMainModel(const std::string& filepath, bool averagedNormals);
	// Environment map. Useful for skyboxes and global illumination.
	Environment& environment() noexcept { return environment_; }
	// Reset model transform, delete current camera, and start using a new default camera
	void reset();
	const float time() const noexcept { return time_; }
private:
	void configurePipeline();
	void processPipeline();
	static constexpr float ZOOM_SPEED = 0.1f;
	Input* input_ = nullptr;
	Environment environment_;
	float time_ = 0.0;
};

