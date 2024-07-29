#pragma once
#include "Bus.h"
#include "Mappers.h"
#include "Joypad.h"
#include "Timer.h"
#include "SPU.h"
#include "PPU.h"
#include "DMA.h"
#include "CPU.h"

class Bus;
class MMC;
class MBC1;
class Joypad;
class Timer;
class SPU;
class PPU;
class DMA;
class CPU;
enum class MMCType;

class GBCEmulator {
public:
	static const uint32_t FrameCycles = 70224;
	static const uint32_t HalfFrameCycles = FrameCycles / 2;
	static const uint32_t QuarterFrameCycles = FrameCycles / 4;

	Bus bus;
	MMC* mmc;
	DMA dma;
	Joypad joypad1;
	Timer timer;
	SPU spu;
	PPU ppu;
	CPU cpu;

	bool romLoaded = false;
	int32_t clockAligner = 0;

	GBCEmulator();

	void Reset();
	bool LoadROM(uint8_t* data, uint32_t romSize);
	void Run(uint32_t cpuCycles);
	bool IsFrameReady();
	void ResetFrameReadyFlag();

private:
	MMC* CreateMMC(MMCType type);
};

