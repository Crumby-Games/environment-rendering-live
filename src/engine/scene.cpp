#include "scene.h"

Scene::Scene(const int width, const int height) :
	width_(width), height_(height),
	pipeline_(width, height) {
	auto& p = pipeline();

	p.createRenderer("fullscreen", "core/fullscreen.vert", false).uploadAttributes(
		{ {
		-1.f, -1.f, 0.f,
		1.f, -1.f, 0.f,
		1.f,  1.f, 0.f,

		-1.f, -1.f, 0.f,
		1.f,  1.f, 0.f,
		-1.f,  1.f, 0.f
	} }
	);
}

Model& Scene::setModel(const std::string& name, const std::string& filepath, bool smoothNormals) {
	if (models_.contains(name)) models_.erase(name);
	models_.emplace(name, new Model(Mesh(filepath, smoothNormals)));
	Model& model = *models_.at(name);

	std::vector<float> positions, normals;
	Mesh& mesh = model.mesh();

	model.mesh().flatten(positions, normals);
	GLRenderer* r = pipeline().findRenderer("m_" + name);
	if (!r)
		r = &pipeline().createRenderer("m_" + name, "app/geometry.vert", true);
	r->uploadAttributes({ positions, normals });
	return model;
}

void Scene::deleteModel(const std::string& name) {
	if (!models_.contains(name)) throw std::runtime_error("Model \"" + name + "\" does not exist to delete.");
	models_.erase(name);
	pipeline().deleteRenderer("m_" + name);
}

void Scene::resizeViewport(int width, int height) {
	width_ = width; height_ = height;
	camera_->aspect() = float(width) / float(height);
	pipeline_.resizeViewport(width, height);
	onResize();
}

void Scene::pullState(Scene& source) {
	for (auto& [name, model] : source.models_) {
		std::vector<float> positions, normals;
		model->mesh().flatten(positions, normals);

		models_.emplace(name,std::move(model));
		GLRenderer* r = pipeline_.findRenderer("m_" + name);
		if (!r)
			r = &pipeline_.createRenderer("m_" + name, "app/geometry.vert", true);
		r->uploadAttributes({ positions, normals });
	}

	if (camera_) camera_ = std::move(source.camera_);
}
