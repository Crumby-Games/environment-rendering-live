#include "mesh.h"
#include <iostream>
#include <filesystem>

#define TINYOBJLOADER_IMPLEMENTATION

using glm::vec3;
using glm::vec2;
namespace tobj = tinyobj;

static vec3 unpackVec3(const std::vector<float>& data, int idx) {
	return { data[3 * idx + 0], data[3 * idx + 1], data[3 * idx + 2] };
}

static void packVec3(std::vector<float>& data, const vec3& v) {
	data.push_back(v.x); data.push_back(v.y); data.push_back(v.z);
}

static constexpr double CHECKSUM_PRECISION = 1e-4;

// Arbitrarily adds the sum of the components to the sum of the components squared, which has an assymetrical result.
// Truncates floats to avoid precision-based issues.
static int calculateChecksum(const vec3& v) {
	int c = int(v.x / CHECKSUM_PRECISION) + int(v.y / CHECKSUM_PRECISION) + int(v.z / CHECKSUM_PRECISION);
	c += int(v.x * v.x / CHECKSUM_PRECISION) + int(v.y * v.y / CHECKSUM_PRECISION) + int(v.z * v.z / CHECKSUM_PRECISION);
	return c;
}

Mesh::Mesh(const std::string& path, bool averageNormals) : averagedNormals_(averageNormals) {
	bool isRelative = std::filesystem::path(path).is_relative();
	const std::string& filepath = isRelative ? MESH_DIR + path : path;
	tobj::ObjReaderConfig config;
	tobj::ObjReader reader;
	config.triangulate = true; // Use TinyObj's automatic mesh triangulation
	if (!reader.ParseFromFile(filepath, config))
		throw std::runtime_error("Failed to parse file.\nPath" + filepath + "\nReason: " + reader.Error());
	if (reader.GetAttrib().vertices.size() % 3 != 0)
		throw std::runtime_error("Loaded OBJ not triangulated correctly.\nPath: " + filepath);
	if (reader.GetAttrib().vertices.size() / 3 == 0)
		throw std::runtime_error("Loaded OBJ has no vertices.\nPath: " + filepath);
	process(reader);
	if (faces_.size() == 0)
		throw std::runtime_error("Loaded OBJ has no faces.\nPath: " + filepath);
}

void Mesh::flatten(std::vector<float>& positionsOut, std::vector<float>& normalsOut) const
{
	positionsOut.clear();
	normalsOut.clear();
	for (const auto& f : faces_) {
		for (const auto& v : { f.v0, f.v1, f.v2 }) {
			packVec3(positionsOut, positions_[v.p]);
			packVec3(normalsOut, normals_[v.n]);
		}
	}
}

void Mesh::process(const tobj::ObjReader& reader) {
	positions_.clear();
	normals_.clear();

	processVertices(reader.GetAttrib().vertices);
	for (auto& shape : reader.GetShapes()) {
		processIndices(shape.mesh.indices);
	}
	
	if (averagedNormals_) {
		for (auto& n : normals_) {
			n = normalize(n);
		}
	}
}

void Mesh::processVertices(const std::vector<float>& packedPositions) {
	vec3 min(std::numeric_limits<float>::max());
	vec3 max(-std::numeric_limits<float>::max());

	if (averagedNormals_) {
		for (size_t i = 0; i < packedPositions.size() / 3; i++) {
			vec3 p = unpackVec3(packedPositions, i);
			min = glm::min(min, p);
			max = glm::max(max, p);
			positions_.push_back(p);
			normals_.push_back(vec3(0.0));
		}
	}
	else {
		for (size_t i = 0; i < packedPositions.size() / 3; i++) {
			vec3 p = unpackVec3(packedPositions, i);
			min = glm::min(min, p);
			max = glm::max(max, p);
			positions_.push_back(p);
		}
	}

	bounds_ = { min, max };

}

void Mesh::processIndices(const std::vector<tobj::index_t>& indices)
{
	if (averagedNormals_) {
		// Since the mesh is triangulated, iterating through the faces can be done by iterating through the indices in triplets.
		for (size_t i = 0; i < indices.size(); i += 3) {
			int i0 = indices[i + 0].vertex_index;
			int i1 = indices[i + 1].vertex_index;
			int i2 = indices[i + 2].vertex_index;
			processSmoothFace(i0, i1, i2);
		}
	}
	else {
		// Since the mesh is triangulated, iterating through the faces can be done by iterating through the indices in triplets.
		for (size_t i = 0; i < indices.size(); i += 3) {
			int i0 = indices[i + 0].vertex_index;
			int i1 = indices[i + 1].vertex_index;
			int i2 = indices[i + 2].vertex_index;
			processFlatFace(i0, i1, i2);
		}
	}
}

void Mesh::processSmoothFace(const int i0, const int i1, const int i2) {
	vec3 p0 = positions_[i0];
	vec3 p1 = positions_[i1];
	vec3 p2 = positions_[i2];

	vec3 e1 = p1 - p0;
	vec3 e2 = p2 - p0;

	vec3 n = cross(e1, e2);

	// Sum normals of faces that connects to the vertices at these indices
	// (each normal will need to be normalised after all smooth faces have been processed)
	normals_[i0] += n;
	normals_[i1] += n;
	normals_[i2] += n;

	faces_.push_back({ {i0, i0}, {i1, i1}, {i2, i2} });

	positionChecksum_ += calculateChecksum(p0);
	positionChecksum_ += calculateChecksum(p1);
	positionChecksum_ += calculateChecksum(p2);
	normalChecksum_ += calculateChecksum(normalize(n));
}

void Mesh::processFlatFace(const int i0, const int i1, const int i2) {
	vec3 p0 = positions_[i0];
	vec3 p1 = positions_[i1];
	vec3 p2 = positions_[i2];

	vec3 e1 = p1 - p0;
	vec3 e2 = p2 - p0;

	vec3 n = normalize(cross(e1, e2));

	// Each vertex will point to the same normal in memory
	int ni = normals_.size();
	normals_.push_back(n);
	faces_.push_back({ {i0, ni}, { i1, ni }, { i2, ni } });

	positionChecksum_ += calculateChecksum(p0);
	positionChecksum_ += calculateChecksum(p1);
	positionChecksum_ += calculateChecksum(p2);
	normalChecksum_ += calculateChecksum(n);
}
