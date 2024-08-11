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
	Bus(MMC* mmc, DMA& dma, Joypad& jooypad1, Timer& timer, SPU& spu, PPU& ppu, CPU& cpu);

	void Reset();
	void TriggerInterruption(Interruption exception);

	uint8_t Read8(uint16_t address, bool isDMAAccess = false);
	void Write8(uint16_t address, uint8_t value, bool isDMAAccess = false);
	uint16_t Read16(uint16_t address);
	void Write16(uint16_t address, uint16_t value);
	OAMEntry* GetOAMEntry(uint8_t index);

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

