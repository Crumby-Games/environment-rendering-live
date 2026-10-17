#pragma once
#include "engine/application.h"
#include "demo_scene.h"
#include "input.h"

class DemoApplication : public Application {
public:
	DemoApplication();
	~DemoApplication();
	int run() override;
	void configureInput();
	DemoScene* demoScene() noexcept { return static_cast<DemoScene*>(scene()); }
private:
	void gui_init();
	void gui_newFrame();
	void gui_build();
	void gui_scale();
	void gui_render();
	void gui_shutdown();
	Pass& gui_buildPass(const std::string& name, bool hasDefault = false);
	Input input_;
	float sidebarWidth_ = 250.0f;
};
