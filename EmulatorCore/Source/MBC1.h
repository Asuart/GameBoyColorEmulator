#pragma once
#include "MMC.h"

class Bus;
class MMC;

class MBC1 : public MMC {
public:
	MBC1(Bus& bus);

	void Reset();
	void LoadROM(uint8_t* data, uint32_t romSize);

	uint8_t Read8(uint16_t address);
	void Write8(uint16_t address, uint8_t value);
	uint8_t ReadHRAM(uint16_t address);
	void WriteHRAM(uint16_t address, uint8_t value);
	uint8_t ReadOAM(uint16_t address);
	void WriteOAM(uint16_t address, uint8_t value);
	OAMEntry* GetOAMEntry(uint8_t index);

private:
	uint8_t ROMBanksCount;
	uint8_t RAMBanksCount;
	bool multicart = false;
	uint8_t RAMEnable;
	uint8_t ROMBank;
	uint8_t RAMBank;
	uint8_t mode;

	std::array<std::array<uint8_t, 0x4000>, 128> romBanks;
	std::array<uint8_t, 0x2000> vram;
	std::array<std::array<uint8_t, 0x2000>, 4> ramBanks;
	std::array<uint8_t, 0x1000> wram0;
	std::array<uint8_t, 0x1000> wram1;
	std::array<uint8_t, MMC::OAMSize> oam;
	std::array<uint8_t, 0x100> hram;

	uint8_t GetROMBank0();
	uint8_t GetROMBank();
	uint8_t GetRAMBank();
};