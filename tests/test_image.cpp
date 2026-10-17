#include <catch.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "engine/image.h"

using Catch::Matchers::ContainsSubstring;

struct ImageTest {
	std::string path;
	int channels;
	int expectedWidth;
	int expectedHeight;
	std::vector<float> expectedValues;
};

static std::string RelativePath(const std::string& path) { return IMG_DIR + std::format("tests/{}",path); }

std::vector<ImageTest> images = {
	ImageTest { "red.png",	3,	512,	512,	{1.0,0.0,0.0,1.0,0.0,0.0}},
	ImageTest { "red.png",	4,	512,	512,	{1.0,0.0,0.0,1.0,1.0,0.0,0.0,1.0}},
	ImageTest { "white.png",4,	512,	512,	{1.0,1.0,1.0,1.0}},
	ImageTest { "tiny.png",	3,	1,	1,	{1.0,1.0,1.0}},
	ImageTest { "compression.png",	3,	2,	2, {
		1.0,0.0,0.0,
		0.0,1.0,0.0,
		0.0,0.0,1.0,
		0.0,0.0,0.0}
	},
	ImageTest { "odd.png",	3,	7,	3, {1.0,1.0,1.0}},
	ImageTest { "transparent-grid.png", 3, 2, 2, {0.0,0.0,0.0,0.0,0.0,0.0} },
	ImageTest { "transparent-grid.png", 4, 2, 2, {0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0} },
};

TEST_CASE("File corruption") {
	REQUIRE_NOTHROW(Image(RelativePath("valid.bmp")));
	REQUIRE_THROWS(Image(RelativePath("corrupt.bmp")));
}

TEST_CASE("Image loading") {
	for (const ImageTest& testData : images) {
		Image img(RelativePath(testData.path), testData.channels);

		SECTION(testData.path + " expected width") {
			REQUIRE(img.width() == testData.expectedWidth);
		}
		SECTION(testData.path + " expected height") {
			REQUIRE(img.height() == testData.expectedHeight);
		}
		SECTION(testData.path + " expected values") {
			REQUIRE(img.data().size() >= testData.expectedValues.size());
			for (int i = 0; i < testData.expectedValues.size(); ++i) {
				REQUIRE(img.data()[i] == Catch::Approx(testData.expectedValues[i]).margin(1e-2f));
			}
		}
	}
}
