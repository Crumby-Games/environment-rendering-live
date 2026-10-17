	#include "demo_scene.h"
#include "orbit_camera.h"

using enum GLFramebuffer::AttachSlot;

void DemoScene::configurePipeline() {
	auto& p = pipeline();

	p.createFBO("gbuffer")
		.generateDefaultAttachment(COLOR0, false)
		.generateDefaultAttachment(COLOR1, false)
		.generateDefaultAttachment(DEPTH, false);

	p.createFBO("lighting")
		.generateDefaultAttachment(COLOR0, true);

	p.createFBO("hdr")
		.generateDefaultAttachment(COLOR0, true);

	p.createFBO("post")
		.generateDefaultAttachment(COLOR0, true);

	p.createPass("geometry", "m_main", "app/geometry.frag")
		.write("gbuffer")
		.set("uAlbedo", glm::vec3(0.7));

	p.createCopyPass("gbuffer")
		.write("lighting");

	p.createPass("light", "fullscreen", "app/lighting.frag")
		.write("lighting")
		.read("uEnvironment", environment().map())
		.read("uPrefiltered", environment().filteredMap())
		.read("uIrradiance", environment().irradianceMap())
		.read("gAlbedo", "gbuffer", COLOR0)
		.read("gNormal", "gbuffer", COLOR1)
		.read("gDepth", "gbuffer", DEPTH)
		.set("uModeFlags", 15)
		.set("uEnvMappingWeight",0.2f)
		.set("uRoughness", 1.0f)
		.set("uSpecularColor", glm::vec3(0.04f))
		.enabled() = false;

	Pass& ssao = p.createPass("ssao", "fullscreen", "app/ssao.frag")
		.write("lighting")
		.read("uColor", "lighting", COLOR0)
		.read("gNormal", "gbuffer", COLOR1)
		.read("gDepth", "gbuffer", DEPTH)
		.read("uNoise", environment_.noise())
		.set("uKernelSize", environment_.kernel().size())
		.set("uIntensity", 1.0f)
		.set("uRadius", 0.3f);

	for (int i = 0; i < environment_.kernel().size(); ++i) {
		ssao.set("uKernel[" + std::to_string(i) + "]", environment_.kernel()[i]); // The API only supports uploading uniform array items one at a time
	}

	p.createPass("clear", "fullscreen", "core/fill.frag")
		.write("hdr")
		.set("uFillColor", glm::vec3(0.2f));
	p.createPass("sky", "skybox", "app/skybox.frag")
		.write("hdr")
		.read("uEnvironment", environment().map())
		.enabled() = false;
	p.createCompositePass("hdr", "lighting")
		.write("hdr");

	p.createCopyPass("hdr")
		.write("post");
	p.createPass("user", "fullscreen", "core/fill.frag") // Placeholder for a user shader before one is imported. Redundancy has minimal impact on load-time and run-time performance.
		.enabled() = false;
	p.createPass("tonemap", "fullscreen", "app/tonemap.frag")
		.write("post")
		.read("uColor", "post", COLOR0)
		.set("uAlgorithmMode", 0)
		.set("uExposure", 1.0f);

	p.createPass("postprocessing", "fullscreen", "app/colorgrading.frag")
		.write("post")
		.read("uColor", "post", COLOR0)
		.set("uBrightness", 0.0f)
		.set("uContrast", 1.f)
		.set("uExposure", 1.f)
		.set("uSaturation", 1.f)
		.set("uTint", glm::vec3(1.0))
		.enabled() = false;
	p.createPass("gamma", "fullscreen", "app/gamma.frag")
		.write("post")
		.read("uColor", "post", COLOR0)
		.set("uGamma", 2.2f);

	p.createCopyPass("post");
}

void DemoScene::onResize() {
	const auto& noise = environment_.noise();
	pipeline().passes().at("ssao").set("uNoiseScale", glm::vec2(float(width_) / noise.width(), float(height_) / noise.height()));
}

DemoScene::DemoScene(const int width, const int height) : Scene(width, height) {
	reset();

	auto& p = pipeline();

	p.createRenderer("m_main", "app/geometry.vert", true);

	p.createRenderer("skybox", "app/skybox.vert", true).uploadAttributes(
		{ {
		-1.f,  1.f, -1.f,  -1.f, -1.f, -1.f,   1.f, -1.f, -1.f,
		1.f, -1.f, -1.f,   1.f,  1.f, -1.f,  -1.f,  1.f, -1.f,

		-1.f, -1.f,  1.f,  -1.f, -1.f, -1.f,  -1.f,  1.f, -1.f,
		-1.f,  1.f, -1.f,  -1.f,  1.f,  1.f,  -1.f, -1.f,  1.f,

		1.f, -1.f, -1.f,   1.f, -1.f,  1.f,   1.f,  1.f,  1.f,
		1.f,  1.f,  1.f,   1.f,  1.f, -1.f,   1.f, -1.f, -1.f,

		-1.f, -1.f,  1.f,  -1.f,  1.f,  1.f,   1.f,  1.f,  1.f,
		1.f,  1.f,  1.f,   1.f, -1.f,  1.f,  -1.f, -1.f,  1.f,

		-1.f,  1.f, -1.f,   1.f,  1.f, -1.f,   1.f,  1.f,  1.f,
		1.f,  1.f,  1.f,  -1.f,  1.f,  1.f,  -1.f,  1.f, -1.f,

		-1.f, -1.f, -1.f,  -1.f, -1.f,  1.f,   1.f, -1.f, -1.f,
		1.f, -1.f, -1.f,  -1.f, -1.f,  1.f,   1.f, -1.f,  1.f
	} }
	);

	configurePipeline();
}

void DemoScene::reset() {
	OrbitCamera* cam = new OrbitCamera(1.0f, 20.0f, 3.0f, float(width_) / height_);
	setCamera(cam);
	if (Model* model = findModel("main")) {
		cam->adjust(model->mesh().boundsRadius());
		model->yaw() = 0;
	}

}

void DemoScene::pullState(Scene& source) {
	Scene::pullState(source);
	DemoScene& demoScene = static_cast<DemoScene&>(source);
	input_ = demoScene.input_;
	environment_ = std::move(demoScene.environment_);
}

void DemoScene::changeMainModel(const std::string& filepath, bool averagedNormals) {
	Model* old = findModel("main");
	float yaw = old ? old->yaw() : 0;
	setModel("main", filepath, averagedNormals);

	Model& model = *findModel("main");
	model.yaw() = yaw;
	float boundsRadius = model.mesh().boundsRadius();
	static_cast<OrbitCamera*>(camera())->adjust(boundsRadius);
	pipeline().passes().at("ssao").material().set("uRadius", boundsRadius);
}

void DemoScene::setUserShader(const std::string& fragShader) {
	try {
		Pass& pass = pipeline().replacePass("user", "user", "fullscreen", fragShader)
			.write("post");
		if (pass.material().shader().hasUniform("uColor")) pass.read("uColor", "post", COLOR0);
		if (pass.material().shader().hasUniform("gAlbedo")) pass.read("gAlbedo", "gbuffer", COLOR0);
		if (pass.material().shader().hasUniform("gNormal")) pass.read("gNormal", "gbuffer", COLOR1);
		if (pass.material().shader().hasUniform("gDepth")) pass.read("gDepth", "gbuffer", DEPTH);
	}
	catch (std::exception& e) {
		pipeline().findPass("user")->enabled() = false;
		throw;
	}
}

void DemoScene::process(float delta) {
	Model* model = findModel("main");
	time_ += delta;
	if (time_ > 1000.0f)
		time_ -= 1000.0f; // prevent precision issues if application runs for a long time
	if (!model) return;

	if (input_) {
		OrbitCamera* oCam = static_cast<OrbitCamera*>(camera());
		if (input_->mouse.activeButton != MouseButton::None) {
			// Normalise change in cursor position
			float ndx = input_->mouse.dx / width_;
			float ndy = input_->mouse.dy / height_;

			// Perform mouse action
			switch (input_->mouse.activeButton) {
			case MouseButton::Right:
				oCam->rotate(ndx * glm::two_pi<float>(), -ndy * glm::pi<float>());
				break;
			case MouseButton::Middle:
				oCam->pan(ndx, ndy);
				break;
			case MouseButton::Left:
				model->yaw() -= ndx * glm::two_pi<float>();
				break;
			}
		}
		// Dolly zoom based on mouse wheel
		if (input_->mouse.dw != 0.0f)
			oCam->dolly(1.0f - input_->mouse.dw * ZOOM_SPEED);
	}

	processPipeline();
}

void DemoScene::processPipeline() {
	auto& p = pipeline();
	
	glm::mat4 view = camera()->view();
	glm::mat4 proj = camera()->projection();
	glm::mat4 model = models().at("main")->matrix();
	glm::mat4 invProj = glm::inverse(proj);
	glm::mat4 invView = glm::inverse(view);

	p.passes().at("geometry")
		.set("uProj", proj)
		.set("uView", view)
		.set("uModel", model);

	p.passes().at("sky")
		.set("uView", view)
		.set("uProj", proj);

	p.passes().at("light")
		.set("uInvProj", invProj)
		.set("uInvView", invView);

	p.passes().at("ssao")
		.set("uInvProj", invProj)
		.set("uProj", proj);

	{
		Pass& pass = p.passes().at("user");
		if (pass.material().shader().hasUniform("uTime"))
			pass.set("uTime", time_);
	}
}
