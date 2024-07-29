#include "Joypad.h"

Joypad::Joypad(Bus& bus) : bus(bus) {}

void Joypad::Reset() {
	data = 0xff;
}

uint8_t Joypad::Read() {
	aOrRight = !(ButtonPressed(Button::A) || ButtonPressed(Button::Right));
	bOrLeft = !(ButtonPressed(Button::B) || ButtonPressed(Button::Left));
	selectOrUp = !(ButtonPressed(Button::Select) || ButtonPressed(Button::Up));
	startOrDown = !(ButtonPressed(Button::Start) || ButtonPressed(Button::Down));
	return data;
}

void Joypad::Write(uint8_t value) {
	data = (value & 0x30) | (data & 0xcf);
}

bool Joypad::ButtonPressed(Button button) {
	if (!buttons[(uint8_t)button]) return false;
	if (((uint8_t)button < 4 && selectButtons) || ((uint8_t)button >= 4 && selectDirections)) {
		return true;
	}
	return false;
}
