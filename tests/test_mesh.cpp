#include <catch.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "engine/mesh.h"

using Catch::Matchers::ContainsSubstring;

struct MeshTest {
	std::string path;
	size_t expectedVertexCount;
	size_t expectedFaceCount;
	uint32_t expectedPositionChecksum;
	uint32_t expectedNormalChecksum;
};

static std::string RelativePath(const std::string& path) { return MESH_DIR + std::format("tests/{}", path); }

std::vector<MeshTest> meshes = {
	MeshTest { "triangle.obj",	3,		1,		53328,		20000	},
	MeshTest { "quad.obj",		4,		2,		120000,		40000	},
	MeshTest { "plane.obj",		4,		2,		120000,		40000	},
	MeshTest { "cube.obj",		8,		12,		1080000,	120000	},
	MeshTest { "pyramid.obj",	5,		6,		560000,		62104	},
	MeshTest { "suzanne.obj",   507,	968,	36666664,	12550692},
};

TEST_CASE(".obj parsing") {
	REQUIRE_THROWS_MATCHES(Mesh("///invalid path?"), std::runtime_error, MessageMatches(ContainsSubstring("cannot open", Catch::CaseSensitive::No)));
	REQUIRE_THROWS_MATCHES(Mesh(RelativePath("path/to/nowhere.obj")), std::runtime_error, MessageMatches(ContainsSubstring("cannot open", Catch::CaseSensitive::No)));
	REQUIRE_THROWS_MATCHES(Mesh(RelativePath("invalid.obj")), std::runtime_error, MessageMatches(ContainsSubstring("no faces", Catch::CaseSensitive::No)));
	REQUIRE_THROWS_MATCHES(Mesh(RelativePath("cube-no-vertices.obj")), std::runtime_error, MessageMatches(ContainsSubstring("no vertices", Catch::CaseSensitive::No)));
	Mesh a(RelativePath("cube.obj"));
	Mesh b(RelativePath("cube-flattened.obj"));
}

TEST_CASE("Mesh shape processing") {
	for (const MeshTest& testData : meshes) {
		Mesh m(RelativePath(testData.path));

		SECTION(testData.path + " vertex count") {
			REQUIRE(m.positions().size() == testData.expectedVertexCount);
		}
		SECTION(testData.path + " face count") {
			REQUIRE(m.faces().size() == testData.expectedFaceCount);
		}
		SECTION(testData.path + " vertex checksum") {
			REQUIRE(m.positionChecksum() == testData.expectedPositionChecksum);
		}
		SECTION(testData.path + " normal checksum") {
			REQUIRE(m.normalChecksum() == testData.expectedNormalChecksum);
		}
	}
}

TEST_CASE("Flattening quad") {
	Mesh q(RelativePath("quad.obj")); // Already triangulated so vertex order will be consistent
	std::vector<float> positions;
	std::vector<float> normals;

	q.flatten(positions, normals);

	const float targetPositions[] = {
		// Triangle 1
		-1, 0, -1,
		 1, 0,  1,
		 1, 0, -1,
		 // Triangle 2
		 -1, 0, -1,
		 -1, 0,  1,
		  1, 0,  1,
	};

	const float targetNormals[] = {
		// Triangle 1
		 0, 1, 0,
		 0, 1, 0,
		 0, 1, 0,
		 // Triangle 2
		  0, 1, 0,
		  0, 1, 0,
		  0, 1, 0,
	};

	const int targetSize = 18;

	REQUIRE(positions.size() == targetSize);
	REQUIRE(normals.size() == targetSize);

	for (int i = 0; i < targetSize; i++) {
		INFO("i = " << i);
		REQUIRE(positions[i] == Catch::Approx(targetPositions[i]));
		REQUIRE(normals[i] == Catch::Approx(targetNormals[i]));
	}
}


