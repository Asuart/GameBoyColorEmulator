#pragma once
#include <cstdint>
#include <vector>

struct Color {
	float r = 0, g = 0, b = 0;
	bool operator==(const Color& other) {
		return r == other.r && g == other.g && b == other.b;
	}
	bool operator!=(const Color& other) {
		return r != other.r || g != other.g || b != other.b;
	}
};

template <typename T>
struct Texture {
	uint32_t width = 0;
	uint32_t height = 0;
	std::vector<T> pixels = std::vector<T>(0);

	Texture(uint32_t _width = 0, uint32_t _height = 0)
		: width(_width), height(_height), pixels(_width * _height) {}

	size_t ByteSize() {
		return pixels.size() * sizeof(T);
	}

	T GetPixel(uint32_t x, uint32_t y) {
		if (x >= width || y >= height) return T();
		uint32_t pixelIndex = y * width + x;
		return pixels[pixelIndex];
	}

	void SetPixel(uint32_t x, uint32_t y, T p) {
		if (x >= width || y >= height) return;
		uint32_t pixelIndex = y * width + x;
		pixels[pixelIndex] = p;
	}

	void AccumulatePixel(uint32_t x, uint32_t y, T p) {
		if (x >= width || y >= height) return;
		uint32_t pixelIndex = y * width + x;
		pixels[pixelIndex] += p;
	}

	void Resize(uint32_t _width, uint32_t _height) {
		width = _width;
		height = _height;
		pixels.resize(width * height);
		Reset();
	}

	void Reset() {
		memset(&pixels[0], 0, ByteSize());
	}
};
