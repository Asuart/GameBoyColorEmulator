#include "SaveState.h"

SaveState::SaveState(const std::string& name)
	: name(name) {}

void SaveState::Write8(uint8_t value) {
	data.push_back(value);
}

void SaveState::Write16(uint16_t value) {
	for (uint32_t i = 0; i < sizeof(value); i++) {
		data.push_back((value >> (i * 8)) & 0xff);
	}
}

void SaveState::Write32(uint32_t value) {
	for (uint32_t i = 0; i < sizeof(value); i++) {
		data.push_back((value >> (i * 8)) & 0xff);
	}
}

void SaveState::Write64(uint64_t value) {
	for (uint32_t i = 0; i < sizeof(value); i++) {
		data.push_back((value >> (i * 8)) & 0xff);
	}
}

uint8_t SaveState::Read8() {
	return data[cursor++];
}

uint16_t SaveState::Read16() {
	uint16_t value = 0;
	for (uint32_t i = 0; i < sizeof(value); i++) {
		value |= data[cursor++] << (8 * i);
	}
	return value;
}

uint32_t SaveState::Read32() {
	uint32_t value = 0;
	for (uint32_t i = 0; i < sizeof(value); i++) {
		value |= data[cursor++] << (8 * i);
	}
	return value;
}

uint64_t SaveState::Read64() {
	uint64_t value = 0;
	for (uint32_t i = 0; i < sizeof(value); i++) {
		value |= data[cursor++] << (8 * i);
	}
	return value;
}
