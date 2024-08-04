#pragma once
#include <cstdint>
#include "Bus.h"

class Bus;

class DMA {
public:
	bool active = false;

	DMA(Bus& bus);

	void Reset();
	void Step(uint32_t cpuCycles);

	uint8_t ReadOAM();
	uint8_t ReadHDMA1();
	uint8_t ReadHDMA2();
	uint8_t ReadHDMA3();
	uint8_t ReadHDMA4();
	uint8_t ReadHDMA5();
	void WriteOAM(uint8_t value);
	void WriteHDMA1(uint8_t value);
	void WriteHDMA2(uint8_t value);
	void WriteHDMA3(uint8_t value);
	void WriteHDMA4(uint8_t value);
	void WriteHDMA5(uint8_t value);

	void WriteState(SaveState& state);
	void LoadState(SaveState& state);

protected:
	Bus& bus;

	uint8_t OAM;
	uint8_t HDMA1;
	uint8_t HDMA2;
	uint8_t HDMA3;
	uint8_t HDMA4;
	uint8_t HDMA5;

	uint32_t writes = 0;

	friend class GBCEmulator;
};

