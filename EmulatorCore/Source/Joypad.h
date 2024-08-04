#pragma once
#include <cstdint>
#include "Bus.h"

class Bus;

enum class Button {
	A = 0,
	B,
	Select,
	Start,
	Right,
	Left,
	Up,
	Down,
	COUNT
};

class Joypad {
public:
	Bus& bus;

	std::array<uint8_t, 8> buttons;
	union {
		struct {
			uint8_t aOrRight : 1;
			uint8_t bOrLeft : 1;
			uint8_t selectOrUp : 1;
			uint8_t startOrDown : 1;
			uint8_t selectButtons : 1;
			uint8_t selectDirections : 1;
		};
		uint8_t data;
	};

	Joypad(Bus& bus);

	void Reset();
	uint8_t Read();
	void Write(uint8_t value);
	bool ButtonPressed(Button button);

	void WriteState(SaveState& state);
	void LoadState(SaveState& state);
};

