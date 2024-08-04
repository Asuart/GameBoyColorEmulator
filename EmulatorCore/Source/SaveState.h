#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

struct SaveState {
	std::string name;
	std::vector<uint8_t> data = std::vector<uint8_t>();
	uint32_t cursor = 0;

	SaveState(const std::string& name);

	void Write8(uint8_t value);
	void Write16(uint16_t value);
	void Write32(uint32_t value);
	void Write64(uint64_t value);

	uint8_t Read8();
	uint16_t Read16();
	uint32_t Read32();
	uint64_t Read64();
};