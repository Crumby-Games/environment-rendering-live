#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>

// Wrapper for a TinyOBJ mesh
class Mesh {
public:
	// Abstraction per vertex that belongs to a face. There are duplicates per position, so it does not own vertex data itself.
	// Stores indices of values in Mesh owner's positions() and normals() respectively 
	// If normals are averaged, each vertex's normal is the average of the normals of the faces it connects to.
	// Otherwise, each vertex's normal is the normal of the face it belongs to.
	struct Vertex { int p, n; };
	// Abstraction per face. Owns vertices.
	struct Face { Vertex v0, v1, v2; };

	// Constructs a mesh from a path to an .obj file
	explicit Mesh(const std::string& path, bool averageNormals = false);

	// Outputs flattened float data for each face.
	// Copies normal data for each vertex.
	// Has duplicate positions for shared vertices between faces.
	// positionsOut and normalsOut will be equal in length (= faces * 9).
	// For each face,
	// [v1x,	v1y,	v1z,	v2x,	v2y,	v2z,	v3x,	v3y,	v3z]
	// [nx,	ny,	nz,	nx,	ny,	nz,	nx,	ny,	nz]
	void flatten(std::vector<float>& positionsOut, std::vector<float>& normalsOut) const;

	// Axis-aligned bounding box (min, max)
	const std::pair<glm::vec3, glm::vec3>& bounds() const noexcept { return bounds_; }
	float boundsRadius() const noexcept { return glm::distance(bounds_.first, bounds_.second) / 2.f; }

	// Gets the arbitrary irreversible value representing all vertex positions (for debugging)
	int positionChecksum() const noexcept { return positionChecksum_; }

	// Gets the arbitrary irreversible value representing all surface normals (for debugging)
	int normalChecksum() const noexcept { return normalChecksum_; }

	// Whether normal smoothing (vertex normals instead of face normals) was used to load the mesh.
	bool averagedNormals() const noexcept { return averagedNormals_; }

	const std::vector<glm::vec3>& positions() const noexcept { return positions_; }
	const std::vector<glm::vec3>& normals() const noexcept { return normals_; }
	const std::vector<Face>& faces() const noexcept { return faces_; }
	const std::string& sourcePath() const noexcept { return filepath_; }
private:
	void process(const tinyobj::ObjReader& reader);
	void processVertices(const std::vector<float>& packedPositions);
	void processIndices(const std::vector<tinyobj::index_t>& indices);
	void processSmoothFace(const int i0, const int i1, const int i2);
	void processFlatFace(const int i0, const int i1, const int i2);
	std::string filepath_;
	std::vector<glm::vec3> positions_;
	std::vector<glm::vec3> normals_;
	std::vector<Face> faces_;
	std::pair<glm::vec3, glm::vec3> bounds_;
	const bool averagedNormals_ = false;
	int positionChecksum_ = 0;
	int normalChecksum_ = 0;
};
