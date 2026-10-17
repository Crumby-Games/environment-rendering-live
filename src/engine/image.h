#pragma once

#include <string>
#include <vector>

class Image {
public:
	// Parses and uploads image to GPU using stb_image.
	// Supports common formats like .jpg, .png, .bmp and .gif.
	// For HDR functionality, supports only .hdr files
	Image(const std::string& path, int channels = 3);
	// Flattened colour channels in sequence.
	// For 3-channel data: {r0, g0, b0, r1, g1, b1}
	// FOr 4-channel data: {r0, g0, b0, a0, r1, g1, b1, a1}
	const std::vector<float>& data() const noexcept { return data_; }
	int width() const noexcept { return width_; }
	int height() const noexcept { return height_; }
private:
	void loadHDR(const std::string& filepath, int channels);
	void loadLDR(const std::string& filepath, int channels);
	std::vector<float> data_;
	int width_ = 0;
	int height_ = 0;
};
