#pragma once
#include <cstdint>

struct OAMEntry {
	uint8_t y;
	uint8_t x;
	uint8_t tile;
	struct {
		uint8_t CGBPalette : 3;
		uint8_t bank : 1;
		uint8_t DMGPalette : 1;
		uint8_t xFlip : 1;
		uint8_t yFlip : 1;
		uint8_t priority : 1;
	};
};