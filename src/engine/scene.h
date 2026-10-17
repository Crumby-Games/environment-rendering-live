#pragma once
#include <memory>
#include <unordered_map>
#include "camera.h"
#include "model.h"
#include "pipeline.h"

// Abstract class that stores data to render.
// Contains the framework to run the rendering pipeline and update the world state.
class Scene {
public:
	virtual ~Scene() = default;
	// Run every frame. Executes rendering pipeline which is configured by child scenes.
	void render() const { pipeline_.execute(); }
	// Deletes mesh data
	void deleteModel(const std::string& name);
	virtual void onResize() {};
	// Passes viewport resize request onto children that need it
	void resizeViewport(int width, int height);
	// Transfers generic Scene state to target
	virtual void pullState(Scene& source);
	// Handles real-time updates that are scene-specific.
	virtual void process(float delta) = 0;

	// Graphics pipeline that can be configured. Has no passes by default.
	// Each scene can have only one.
	Pipeline& pipeline() noexcept { return pipeline_; }
	const std::unordered_map<std::string, std::unique_ptr<Model>>& models() const noexcept { return models_; }
	std::unordered_map<std::string, std::unique_ptr<Model>>& models() noexcept { return models_; }
	// Returns model stored by name, if any. Returns nullptr if no such model exists.
	Model* findModel(const std::string& name) noexcept { return models_.contains(name) ? models_.at(name).get() : nullptr; }
	// Replaces or creates model by name
	Model& setModel(const std::string& name, const std::string& filepath, bool smoothNormals);
	// Camera instance to be used for calculating view and projection matrices. Can be nullptr.
	Camera* camera() noexcept { return camera_.get(); }
	// Camera instance to be used for calculating view and projection matrices. Takes ownership.
	void setCamera(Camera* cam) { camera_.reset(cam); }
	// Width of the scene viewport in pixels
	int width() noexcept { return width_; }
	// Height of the scene viewport in pixels
	int height() noexcept { return height_; }
protected:
	Scene(const int width, const int height);
	int width_, height_;
private:
	std::unique_ptr<Camera> camera_ = nullptr;
	std::unordered_map<std::string, std::unique_ptr<Model>> models_;
	Pipeline pipeline_;

};
