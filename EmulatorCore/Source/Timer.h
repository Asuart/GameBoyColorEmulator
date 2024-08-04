#pragma once
#include <cstdint>
#include "Bus.h"

class Bus;

class Timer {
public:
	Timer(Bus& bus);

	void Reset();
	void Step(uint32_t cycles);

	uint8_t ReadDIV();
	uint8_t ReadTIMA();
	uint8_t ReadTMA();
	uint8_t ReadTAC();
	void WriteDIV(uint8_t value);
	void WriteTIMA(uint8_t value);
	void WriteTMA(uint8_t value);
	void WriteTAC(uint8_t value);

	void WriteState(SaveState& state);
	void LoadState(SaveState& state);

private:
	Bus& bus;
	uint8_t DIV;
	uint8_t TIMA;
	uint8_t TMA;
	uint8_t TAC;
	uint32_t DIVClockAccumulator;
	uint32_t TIMAClockAccumulator;
};

