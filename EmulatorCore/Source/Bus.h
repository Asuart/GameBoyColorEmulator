#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include "Mappers.h"
#include "Joypad.h"
#include "Timer.h"
#include "SPU.h"
#include "PPU.h"
#include "DMA.h"
#include "CPU.h"
#include "OAMEntry.h"

class GBCEmulator;
class MMC;
class Joypad;
class Timer;
class SPU;
class PPU;
class DMA;
class CPU;

enum class Interruption {
	VBlank = 0b1,
	LCD = 0b10,
	Timer = 0b100,
	Serial = 0b1000,
	Joypad = 0b10000
};

class Bus {
public:
	union {
		struct {
			uint8_t ifVBlank : 1;
			uint8_t ifLCD : 1;
			uint8_t ifTimer : 1;
			uint8_t ifSerial : 1;
			uint8_t ifJoypad : 1;
			uint8_t ifUnused : 3;
		};
		uint8_t IF; // Interruption Flag
	};
	union {
		struct {
			uint8_t ieVBlank : 1;
			uint8_t ieLCD : 1;
			uint8_t ieTimer : 1;
			uint8_t ieSerial : 1;
			uint8_t ieJoypad : 1;
			uint8_t ieUnused : 3;
		};
		uint8_t IE; // Interrupt Enable
	};
	uint8_t key1;

	Bus(MMC* mmc, DMA& dma, Joypad& jooypad1, Timer& timer, SPU& spu, PPU& ppu, CPU& cpu);

	void Reset();
	void TriggerInterruption(Interruption exception);

	uint8_t Read8(uint16_t address, bool isDMAAccess = false);
	void Write8(uint16_t address, uint8_t value, bool isDMAAccess = false);
	uint16_t Read16(uint16_t address);
	void Write16(uint16_t address, uint16_t value);
	OAMEntry* GetOAMEntry(uint8_t index);
	void TriggerSPUTick();

	MMC* mmc;
	DMA& dma;
	Joypad& joypad1;
	Timer& timer;
	SPU& spu;
	PPU& ppu;
	CPU& cpu;

private:
	friend class GBCEmulator;
};

