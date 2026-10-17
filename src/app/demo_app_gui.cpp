#include "demo_app.h"
#include <functional>
#include <algorithm>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "gui/helpers.h"
#include "gui/widgets.h"
#include "orbit_camera.h"

using namespace GUI;

static void buildRendererInfo(const char* name, const GLRenderer& r) {
	if (ImGui::TreeNode(name)) {
		Label("Has Vertices");
		ImGui::TextWrapped(r.hasData() ? "Yes" : "No");

		Label("Depth Testing");
		ImGui::TextWrapped(r.depthTesting() ? "Enabled" : "Disabled");

		Label("Wireframe Mode");
		ImGui::TextWrapped(r.wireframe() ? "Enabled" : "Disabled");

		Label("Viewport Position");
		ImGui::TextWrapped("(%d, %d)", r.viewport()[0], r.viewport()[1]);

		Label("Viewport Size");
		ImGui::TextWrapped("(%d, %d)", r.viewport()[2], r.viewport()[3]);

		Label("Clear Colour");
		ImGui::TextWrapped("(%f, %f, %f, %f)", r.clearColor()[0], r.clearColor()[1], r.clearColor()[2], r.clearColor()[3]);

		ImGui::TreePop();
	}
}

Pass& DemoApplication::gui_buildPass(const std::string& name, bool hasDefault) {
	Pass* pass = scene()->pipeline().findPass(name);
	if (pass == nullptr) throw std::runtime_error(std::format("Could not build GUI for pass: {}", name));

	Label("Enabled");
	{
		std::string id = name + "-enabled";
		Widget w(id.c_str(), pass->enabled());
		Tooltip("If disabled, this pass is skipped every frame.");
		w.Checkbox();
		if (hasDefault) w.DefaultValue(true);
	}
	return *pass;
}

void DemoApplication::gui_build() {
	if (StaticSidebarWindow("Properties", &sidebarWidth_)) {
		scene()->resizeViewport(glfw().width() - sidebarWidth_, glfw().height());
	}
	static std::optional<std::exception> displayError;

	static bool averagedMeshNormals = false;

	static ImportData meshImport(
		displayError,
		[&](const std::string& filepath) { demoScene()->changeMainModel(filepath, averagedMeshNormals); },
		MESH_DIR + std::string("tests/cube.obj")
	);

	meshImport.nextState();
	Pipeline& p = scene()->pipeline();

	static ImportData shaderImport(
		displayError,
		[&](const std::string& filepath) {
			Pass* pass = p.findPass("user");
			assert(pass != nullptr);
			if (!filepath.empty())
				demoScene()->setUserShader(filepath);
			pass->enabled() = !filepath.empty();
		},
		""
	);

	shaderImport.nextState();

	static ImportData envImport(
		displayError,
		[&](const std::string& filepath) {
			if (filepath.empty()) {
				demoScene()->environment().reset();
				if (Pass* pass = p.findPass("sky"))
					pass->enabled() = false;
				if (Pass* pass = p.findPass("light"))
					pass->enabled() = false;
			}
			else demoScene()->environment().load(filepath);
		},
		""
	);

	envImport.nextState();

	


	if (displayError) {
		static bool openErrorModal = false;
		if (!openErrorModal) {
			ImGui::OpenPopup("Error");
			openErrorModal = true;
		}

		if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text(displayError->what());
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
				displayError.reset();
				openErrorModal = false;
			}
			ImGui::EndPopup();
		}
	}

	{
		float oldScale = GUI::scaleFactor;
		Label("GUI Scale");
		Tooltip("Scales all interface elements, including text size and widget spacing.");
		Widget w("gui-scale", GUI::scaleFactor);

		bool changed = w.Dropdown({
			{0.25f, "25%"},
			{0.5f, "50%"},
			{0.75f, "75%"},
			{1.f, "100%"},
			{1.25f, "125%"},
			{1.5f, "150%"},
			{2.f, "200%"},
			{3.f, "300%"},
			{4.f, "400%"}
			});
		changed |= w.DefaultValue(1.f);
		if (changed) {
			sidebarWidth_ *= GUI::scaleFactor / oldScale;
			gui_scale();
		};
	}


	if (ImGui::CollapsingHeader("Scene")) {
		ImGui::Indent();
		if (WideButton("Scene Transform"))
			demoScene()->reset();
		Tooltip("Reset camera position, orientation and model transform to their default states.");

		Label("Camera FOV");
		Tooltip("Field of view (vertical) in degrees.");
		{
			Widget w("fovy", scene()->camera()->fov());
			w.Slider(20.0f, 120.0f);
			w.DefaultValue(60.0f);
		}
		ImGui::Unindent();
	}

	if (ImGui::CollapsingHeader("Assets")) {
		ImGui::Indent();

		ImGui::SeparatorText("Mesh");
		ImGui::Indent();
		Tooltip("The 3D object used as the scene's main render object.");
		{
			Label("File");
			Tooltip("3D object file to import mesh data from.\nAccepts only .obj files, and discards existing normal and texcoord data.");
			Widget w("Import Mesh", meshImport.pendingPath);
			if (w.FilePicker(".obj", MESH_DIR) || (meshImport.defaultPath && w.DefaultValue(meshImport.defaultPath.value()))) {
				meshImport.load();
			}
		}
		{
			Label("Smooth Normals");
			Tooltip("Computes vertex normals by averaging adjacent face normals for smoother shading.\n(Reimports mesh with new normals)");
			if (Widget("normal-smoothing", averagedMeshNormals).Checkbox()) {
				meshImport.load();
			}
		}
		ImGui::Unindent();

		ImGui::SeparatorText("Custom Shader");
		Tooltip(
			"Screen-wide fragment shader injected just before post processing steps, including tonemapping. Uniforms are not exposed, but common use cases are bound automatically."
			"\n\nVaryings:"
			"\n   layout (location = 0) in vec2 vUV;"
			"\n        - Screen UV: (0, 0) is bottom-left, (1, 1) is top-right."
			"\n   layout (location = 0) out vec4 oColor;"
			"\n        - Output colour. Alpha is ignored."
			"\n\nAvailable Uniforms:"
			"\n   uColor: Scene colour before post processing"
			"\n   gNormal: Geometry normal buffer"
			"\n   gAlbedo: Geometry albedo buffer"
			"\n   gDepth: Geometry depth buffer (1.0 everywhere else)"
		);
		ImGui::Indent();
		{
			Label("File");
			Tooltip("GLSL fragment shader. Must be .frag.");
			Widget w("Import Shader", shaderImport.pendingPath);
			if (w.FilePicker(".frag", SHADER_DIR) || (shaderImport.defaultPath && w.DefaultValue(shaderImport.defaultPath.value()))) {
				shaderImport.load();
			}
		}
		ImGui::Unindent();
			
		ImGui::SeparatorText("Environment");
		ImGui::Indent();
		Tooltip("Environment map sampled for image-based lighting, environment mapping and skybox.");
		{
			Label("File");
			Tooltip("Can be LDR (.png or .jpg) but HDR (.hdr) is recommended.");
			Widget w("Import Environment", envImport.pendingPath);
			if (w.FilePicker(".hdr,.jpg,.png", IMG_DIR) || (envImport.defaultPath && w.DefaultValue(envImport.defaultPath.value()))) {
				envImport.load();
			}

		}
		if (envImport.pendingPath.empty()) ImGui::BeginDisabled();
		WideLabel("Precomputed Resources");
		Tooltip("There are two maps that are generated in advance to optimise IBL: a prefiltered environment map and an irradiance map.");
		{
			ImGui::Indent();
			Environment& env = demoScene()->environment();

			Label("Roughness");
			Tooltip("Controls blur level used in the prefiltered map, which is also factored directly in the IBL pass."
				"\n(Recomputes prefiltered map)");
			{
				static float roughness = 1.0f;
				Widget w("env-roughness", roughness);
				bool changed = w.Slider(0.0f, 1.0f, false, "%.1f");
				changed |= w.DefaultValue(1.0f);
				if (changed) {
					env.applyRoughness(roughness);
					Pass* lighting = p.findPass("light");
					assert(lighting != nullptr);
					lighting->material().set("uRoughness", env.roughness());
				}
			}

			Label("Level of Detail");
			Tooltip("Mip level of the environment map for sampling in the irradiance map and prefiltered map. Higher values even outs streaks and blotches in IBL."
				"\n(Recomputes prefiltered and irradiance maps)");
			{
				static int lod = 0;
				lod = env.lod();
				if (Widget("env-lod", lod).Slider(0, env.map().mipLevels())) {
					env.applyLod(lod);
				}
			}

			Label("Resolution");
			Tooltip("Height of precomputed textures. Width is half of height."
				"\n(Recomputes prefiltered and irradiance maps)");
			{
				static int res = 512;
				Widget w("env-resolution", res);
				bool changed = w.Slider(64, 2048, true);
				changed |= w.DefaultValue(512);
				if (changed)
					env.applyResolution(res);
			}

			ImGui::Unindent();
		}
		ImGui::Unindent();
		if (envImport.pendingPath.empty()) ImGui::EndDisabled();

		ImGui::Unindent();
	}

	if (ImGui::CollapsingHeader("Passes")) {
		Environment& env = demoScene()->environment();

		ImGui::Indent();
		Label("Clear Colour");
		Tooltip("Colour used to flush the buffers before passes");
		{
			Pass* clearpass = p.findPass("clear");
			assert(clearpass != nullptr);
			Widget w("clear-colour", *clearpass->material().get("uFillColor"));

			w.ColorRGB();
			w.DefaultValue(glm::vec3(0.2f));
		}

		ImGui::SeparatorText("Geometry");
		Tooltip("Draws geometry data into the g-buffer, for use in deferred shading.");
		ImGui::Indent();
		{
			Pass& pass = gui_buildPass("geometry");
			{
				Label("Diffuse Colour");
				Tooltip("Base surface colour of the model. Also known as the albedo.");
				Widget w("albedo", *pass.material().get("uAlbedo"));
				w.ColorRGB();
				w.DefaultValue(glm::vec3(0.7));
			}
		}
		ImGui::Unindent();

		if (envImport.pendingPath.empty()) ImGui::BeginDisabled();
		ImGui::SeparatorText("Skybox");
		Tooltip("The environment map sampled as a cube with infinitely distance faces.");

		ImGui::Indent();
		{
			Pass& skypass = gui_buildPass("sky");
		}
		ImGui::Unindent();
		
		ImGui::SeparatorText("Environment Lighting");
		Tooltip("Coloured ambient lighting that is calculated using the environment map.");
		ImGui::Indent();
		{
			Pass& pass = gui_buildPass("light");
			if (!pass.enabled()) ImGui::BeginDisabled();
			{
				int& flags = *valuePtr<int>(*pass.material().get("uModeFlags"));

				ImGui::SeparatorText("Composition");
				Label("Scattering");
				Tooltip("If disabled, only the specular colour is used. This implicitly disables Lambertian Reflectance.");
				static bool useReflectivity = true;
				if (Widget("use-reflectivity", useReflectivity).Checkbox()) {
					if (useReflectivity) flags |= 1 << 3;
					else flags &= ~(1 << 3);
				}
				if (!useReflectivity) ImGui::BeginDisabled();
				Label("Specular Colour");
				{
					Widget w("spec-colour", *pass.material().get("uSpecularColor"));
					w.ColorRGB();
					w.DefaultValue(glm::vec3(0.04));
				}
				

				ImGui::Indent();

				static bool lambert = true;
				ImGui::SeparatorText("Lambertian Reflectance");
				Label("Enabled");
				if (Widget("lambert-enabled", lambert).Checkbox()) {
					if (lambert) flags |= 1 << 0;
					else flags &= ~(1 << 0);
				}
				if (!useReflectivity) ImGui::EndDisabled();

				static bool ibl = true;
				ImGui::SeparatorText("Image-Based Lighting");
				Label("Enabled");
				if (Widget("ibl-enabled", ibl).Checkbox()) {
					if (ibl) flags |= 1 << 1;
					else flags &= ~(1 << 1);
				}

				static bool envmapping = true;
				ImGui::SeparatorText("Environment Mapping");
				Label("Enabled");
				if (Widget("envmapping-enabled", envmapping).Checkbox()) {
					if (envmapping) flags |= 1 << 2;
					else flags &= ~(1 << 2);
				}

				if (!envmapping || !ibl) ImGui::BeginDisabled();
				{
					Label("Specular Weight");
					Tooltip("What proportion of specular light is from environment mapping");
					Widget w("envmapping-weight", *pass.material().get("uEnvMappingWeight"));
					w.Slider(0.0f, 1.0f, true);
					w.DefaultValue(0.2f);
				}
				if (!envmapping || !ibl) ImGui::EndDisabled();

				ImGui::Unindent();
			}
			if (!pass.enabled()) ImGui::EndDisabled();
		}
		ImGui::Unindent();
		if (envImport.pendingPath.empty()) ImGui::EndDisabled();

		ImGui::SeparatorText("Ambient Occlusion");
		Tooltip("Occlusion of ambient light based on only the geometry itself. Uses a screen-space approximation.");
		ImGui::Indent();
		{
			Pass& pass = gui_buildPass("ssao");
			if (!pass.enabled()) ImGui::BeginDisabled();
			{
				Label("Intensity");
				Tooltip("Blend weight between the resulting occlusion factor and 1 (where 1 is no occlusion).");
				Widget w("ssao-intensity", *pass.material().get("uIntensity"));
				w.Slider(0.0f, 2.f, true);
				w.DefaultValue(1.0f);
			}
			{
				float aabbSize = scene()->models().at("main")->mesh().boundsRadius();
				Label("Radius");
				Tooltip("Distance around each fragment to consider for occlusion. Automatically scaled with mesh size.");
				Widget w("ssao-radius", *pass.material().get("uRadius"));
				w.Slider(aabbSize / 4.f, aabbSize * 2.f, true);
				w.DefaultValue(aabbSize);


			}
			if (!pass.enabled()) ImGui::EndDisabled();
		}
		ImGui::Unindent();

		if (shaderImport.pendingPath.empty()) ImGui::BeginDisabled();
		ImGui::SeparatorText("Custom Shader");
		ImGui::Indent();
		{
			Pass& pass = gui_buildPass("user");
		}
		ImGui::Unindent();
		if (shaderImport.pendingPath.empty()) ImGui::EndDisabled();

		ImGui::SeparatorText("Tone Mapping");
		Tooltip("Converts from HDR to LDR, ensuring no clamping of colour channels.");
		ImGui::Indent();
		{
			Pass& pass = gui_buildPass("tonemap");

			if (!pass.enabled()) ImGui::BeginDisabled();
			{
				Label("Exposure");
				Tooltip("Brightness multiplier before tone mapping is applied, scaling the overall brightness of the image. Functions the same as the \"exposure\" colour-grading parameter, but the result is distinct because this is applied before tone mapping.");
				{
					Widget w("exposure", *pass.material().get("uExposure"));
					w.Slider(0.001f, 10.f, true);
					w.DefaultValue(1.0f);
				}

				
				Label("Algorithm");
				Tooltip("Reinhard occasionally produces less saturated darks, but does not clamp the output regardless of exposure."
				"\n\nACES tone mapping produces better saturation, but is susceptible to clamping artifacts when a fixed exposure is used. Uses the Narkoicz approximation of the curve.");
				{
					Widget w("tonemap-algorithm", *pass.material().get("uAlgorithmMode"));
					w.Dropdown({
						{0, "Reinhard"},
						{1, "ACES (approximation)"}
						});
					w.DefaultValue(0);
				}

			}
			if (!pass.enabled()) ImGui::EndDisabled();
		}
		ImGui::Unindent();

		ImGui::SeparatorText("Colour Grading");
		Tooltip("Operations performed per fragment.");
		ImGui::Indent();
		{
			Pass& pass = gui_buildPass("postprocessing");

			if (!pass.enabled()) ImGui::BeginDisabled();
			{
				Label("Exposure");
				Tooltip("Scales the overall brightness of the image.\n\nEach channel is multiplied by the value");
				Widget w("postexposure", *pass.material().get("uExposure"));
				w.Slider(0.001f, 10.f, true);
				w.DefaultValue(1.0f);
			}
			{
				Label("Brightness");
				Tooltip("Makes the result uniformly lighter or darker.\n\nThe value is added to each channel.");
				Widget w("brightness", *pass.material().get("uBrightness"));
				w.Slider(-1.0f, 1.0f);
				w.DefaultValue(0.0f);
			}
			{
				Label("Contrast");
				Tooltip("Increases or decreases the differences between light and dark areas.\n\nThe value multiplies the deviation from 0.5 for each channel.");
				Widget w("contrast", *pass.material().get("uContrast"));
				w.Slider(0.0f, 2.0f, true);
				w.DefaultValue(1.0f);
			}
			{
				Label("Saturation");
				Tooltip("Controls colour intensity, where 0 is greyscale, and 1 is the original colour.\n\nThe value is the blend weight between the luminance and the original colour.");
				Widget w("saturation", *pass.material().get("uSaturation"));
				w.Slider(0.0f, 2.0f, true);
				w.DefaultValue(1.0f);
			}
			{
				Label("Tint");
				Tooltip("Applies a colour tint to the image applied to the overall result.\n\nEach channel is multiplied by each corresponding channel of the value.");
				Widget w("tint", *pass.material().get("uTint"));
				w.ColorRGB();
				w.DefaultValue(glm::vec3(1.0));
			}

			if (!pass.enabled()) ImGui::EndDisabled();

		}
		ImGui::Unindent();

		ImGui::SeparatorText("Gamma Correction");
		Tooltip("Performs nonlinear brightness correction for display output, because human vision perceives brightness differently to how colour is stored and calculated in rendering."
			"\nApproximation of sRGB luminance decoding.");
		ImGui::Indent();
		{
			Pass& pass = gui_buildPass("gamma");

			if (!pass.enabled()) ImGui::BeginDisabled();
			{
				Label("Gamma");
				Tooltip("The exponent used in the formula. Higher values means a lower contrast as dark regions and midtones become brighter.");
				Widget w("gamma", *pass.material().get("uGamma"));
				w.Slider(0.001f, 10.f, true);
				w.DefaultValue(1.0f);
			}
			if (!pass.enabled()) ImGui::EndDisabled();

		}
		ImGui::Unindent();

		ImGui::Unindent();
	}


	static int skyboxMode = 0;

	if (ImGui::CollapsingHeader("Debugging")) {
		ImGui::Indent();
		if (ImGui::TreeNode("Controls")) {
			Label("Skybox Texture");
			{
				Pass* pass = p.findPass("sky");
				assert(pass != nullptr);

				Widget w("skybox-texture", skyboxMode);
				bool changed = w.Dropdown({
					{ 0, "Environment Map"},
					{ 1, "Irradiance Map"},
					{ 2, "Filtered Environment Map"},
					});
				changed |= w.DefaultValue(0);

				if (changed) {
					Environment& env = demoScene()->environment();
					switch (skyboxMode) {
					case 0: pass->read("uEnvironment", env.map()); break;
					case 1: pass->read("uEnvironment", env.irradianceMap()); break;
					case 2: pass->read("uEnvironment", env.filteredMap()); break;
					}
				}
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Info")) {
			Model* model = scene()->findModel("main");
			if (model && ImGui::TreeNode("Model")) {
				if (ImGui::TreeNode("Mesh")) {
					const Mesh& mesh = model->mesh();
					if (ImGui::TreeNode("Faces")) {
						int iterations = std::min(int(mesh.faces().size()), 12);
						if (mesh.averagedNormals()) {
							for (size_t i = 0; i < iterations; ++i) {
								auto& f = mesh.faces()[i];
								ImGui::SeparatorText(std::format("Face {}", i + 1).c_str());
								Label("Position"); Vec3Display(mesh.positions()[f.v0.p]);
								Label("Normal"); Vec3Display(mesh.normals()[f.v0.n]);
								ImGui::Separator();
								Label("Position"); Vec3Display(mesh.positions()[f.v1.p]);
								Label("Normal"); Vec3Display(mesh.normals()[f.v1.n]);
								ImGui::Separator();
								Label("Position"); Vec3Display(mesh.positions()[f.v2.p]);
								Label("Normal"); Vec3Display(mesh.normals()[f.v2.n]);
							}
						}
						else {
							for (size_t i = 0; i < iterations; ++i) {
								auto& f = mesh.faces()[i];
								ImGui::SeparatorText(std::format("Face {}", i + 1).c_str());
								Label("Position"); Vec3Display(mesh.positions()[f.v0.p]);
								Label("Position"); Vec3Display(mesh.positions()[f.v1.p]);
								Label("Position"); Vec3Display(mesh.positions()[f.v2.p]);
								ImGui::Separator();
								Label("Normal"); Vec3Display(mesh.normals()[f.v0.n]);
							}
						}

					
						if (iterations < mesh.positions().size())
							ImGui::TextDisabled("...");
						ImGui::TreePop();
					}

					Label("Source Path");
					Tooltip("Path to .obj file that is the source of data.");
					ImGui::TextWrapped("%s",meshImport.path.c_str());

					Label("Faces");
					Tooltip("Number of triangular faces in mesh.");
					ImGui::TextWrapped("%d", mesh.faces().size());

					Label("Unique Vertices");
					Tooltip("Number of unique vertices in mesh.");
					ImGui::TextWrapped("%d", mesh.positions().size());

					Label("Bounds");
					Tooltip("The minimum and maximum points of the axis-aligned bounding box of the mesh.");
					Vec3Display(mesh.bounds().first);
					Label("");
					Vec3Display(mesh.bounds().second);

					Label("Position Checksum");
					ImGui::TextWrapped("%d", mesh.positionChecksum());

					Label("Normal Checksum");
					ImGui::TextWrapped("%d", mesh.normalChecksum());

					ImGui::TreePop();
				}

				Label("Rotation");
				Tooltip("Rotation in degrees around the y axis (yaw).");
				ImGui::TextWrapped("%.1f", prettyAngle(model->yaw()));

				Label("Position");
				Tooltip("Offset of model from origin in world space.");
				Vec3Display(model->position());

				ImGui::SeparatorText("Transform");
				Tooltip("Matrix representing object transform.");
				Mat4Table("transform-mat", model->matrix());
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Camera")) {
				const OrbitCamera& cam = *static_cast<OrbitCamera*>(scene()->camera());
				Label("Distance");
				Tooltip("Distance from target in world-space.");
				ImGui::TextWrapped("%.1f", cam.distance());

				Label("Distance Range");
				Tooltip("Minimum and maximum distance from target in world-space.");
				ImGui::TextWrapped("(%.1f, %.1f)", cam.distanceMin(), cam.distanceMax());

				Label("Yaw");
				Tooltip("Rotation (in degrees) around the y axis (up). In other words, angle for looking left/right.");
				ImGui::TextWrapped("%.1f", prettyAngle(cam.yaw()));

				Label("Pitch");
				Tooltip("Rotation (in degrees) around the local z axis (right). In other words, angle for looking up/down.");
				ImGui::TextWrapped("%.1f", prettyAngle(cam.pitch()));

				Label("FOV");
				Tooltip("Field of view (vertical). Angle between top and bottom of the view frustum.");
				ImGui::TextWrapped("%.1f", cam.fov());

				Label("Aspect Ratio");
				Tooltip("Ratio of viewport size (used in projection).");
				ImGui::TextWrapped("%.1f", cam.aspect());

				Label("Panning Radius");
				Tooltip("Radius of spherically bound area. Calculated based on the model's AABB bounding box.");
				ImGui::TextWrapped("%.1f", cam.boundsRadius());

				Label("View Planes");
				Tooltip("Distance to near and far clipping planes in projection. Calculated based on the spherically bound area of the model and the camera's distance from it.");
				ImGui::TextWrapped("(%.1f, %.1f)", cam.near(), cam.far());

				Label("Target");
				Tooltip("Origin point of orbit in world-space.");
				Vec3Display(cam.target());

				Label("Forward");
				Tooltip("Normalised direction of camera. Local x axis.");
				Vec3Display(cam.forward());

				ImGui::SeparatorText("Projection");
				Tooltip("Matrix representing screen projection.");
				Mat4Table("proj-mat", cam.projection());

				ImGui::SeparatorText("View");
				Tooltip("Matrix representing camera transform.");
				Mat4Table("view-mat", cam.view());

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Global Rendering State")) {
				for (const auto& [name, r] : scene()->pipeline().renderers())
					buildRendererInfo(name.c_str(), r);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Miscellaneous")) {
				Label("FPS");
				Tooltip("Frames per second.");
				ImGui::TextWrapped("%.1f", 1.0f / glfw().frameDelta());

				Label("Screen Resolution");
				Tooltip("Width and height of window in pixels.");
				ImGui::TextWrapped("(%d, %d)", glfw().width(), glfw().height());

				Label("Viewport Resolution");
				Tooltip("Width and height of the Scene viewport in pixels.");
				ImGui::TextWrapped("(%d, %d)", scene()->width(), scene()->height());

				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		ImGui::Unindent();
	}

	ImGui::End();
}

static ImGuiStyle baseStyle;

void DemoApplication::gui_scale() {
	ImGuiStyle& style = ImGui::GetStyle();
	style = baseStyle;
	style.ScaleAllSizes(GUI::scaleFactor * GUI::scaleBase);
	style.FontScaleMain = GUI::scaleFactor;
}

void DemoApplication::gui_init() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();
	baseStyle = ImGui::GetStyle();

	glfw().onContentScaleChanged(true, [this](float scaleX, float scaleY) {

		GUI::scaleBase = scaleX;

		sidebarWidth_ = scale(SIDEBAR_INITIAL_WIDTH);
		scene()->resizeViewport(glfw().width() - sidebarWidth_, glfw().height());

		ImGuiIO& io = ImGui::GetIO();
		ImFontConfig fontConfig;
		fontConfig.SizePixels = BASE_FONT_SIZE * GUI::scaleBase;
		io.Fonts->Clear();
		io.Fonts->AddFontDefault(&fontConfig);

		gui_scale();
		}
	);

	ImGui_ImplGlfw_InitForOpenGL(glfw().window(), true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

void DemoApplication::gui_shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void DemoApplication::gui_newFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void DemoApplication::gui_render() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
