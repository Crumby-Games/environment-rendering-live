#include "image.h"
#include <vector>
#include <stdexcept>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Image::Image(const std::string& path, int channels) {
	bool isRelative = std::filesystem::path(path).is_relative();
	const std::string& filepath = isRelative ? IMG_DIR + path : path;
	stbi_set_flip_vertically_on_load(true);
	bool isHDR = stbi_is_hdr(filepath.c_str());
	if (isHDR) loadHDR(filepath, channels);
	else loadLDR(filepath, channels);
}

void Image::loadHDR(const std::string& filepath, int channels) {
	float* data = stbi_loadf(filepath.c_str(), &width_, &height_, nullptr, channels);
	if (!data) throw std::runtime_error("Failed to load HDR image: " + filepath);
	data_.assign(data, data + width_ * height_ * 3);
	stbi_image_free(data);
}

void Image::loadLDR(const std::string& filepath, int channels) {
	unsigned char* data = stbi_load(filepath.c_str(), &width_, &height_, nullptr, channels);
	if (!data) throw std::runtime_error("Failed to load LDR image: " + filepath);

	// LDR images will typically have channels ranging from 0 to 255. However, to normalise the data type, we will convert into a value between 0 and 1.
	data_.resize(width_ * height_ * 3);
	for (int i = 0; i < width_ * height_ * 3; ++i)
		data_[i] = data[i] / 255.0f;
	stbi_image_free(data);
}
