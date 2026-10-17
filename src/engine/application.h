#pragma once
#include <string>
#include <concepts>
#include "scene.h"
#include "glfw_context.h"

// Stores all runtime data and objects
// (Only one can exist concurrently)
class Application {
public:
	// Starts runtime rendering loop
	virtual int run() = 0;

	// Creates and integrate new Scene of the given type
	template <std::derived_from<Scene> SceneType>
	void instantiateScene(int width, int height) { scene_.reset(new SceneType(width, height)); }

	// Switch scene while preserving state
	template <std::derived_from<Scene> SceneType>
	void changeScene() {
		SceneType* newScene = new SceneType(scene_->width(), scene_->height());
		newScene->pullState(*scene_.get());
		scene_.reset(newScene);
	}

	// GLFW instance. Cannot be moved or copied
	GLFWContext& glfw() noexcept { return glfw_; }
	// Currently active scene if any
	Scene* scene() noexcept { return scene_.get(); }
protected:
	Application() {}
private:
	GLFWContext glfw_;
	std::unique_ptr<Scene> scene_ = nullptr;
};
