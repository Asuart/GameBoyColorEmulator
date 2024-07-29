#pragma once
#include <cstdint>
#include <array>
#include <string>
#include <iostream>
#include "OAMEntry.h"

class Bus;

enum class MMCType {
	ROM_ONLY = 0x00,
	MBC1 = 0x01,
	MBC1_RAM = 0x02,
	MBC1_RAM_BATTERY = 0x03,
	MBC2 = 0x05,
	MBC2_BATTERY = 0x06,
	ROM_RAM = 0x08,
	ROM_RAM_BATTERY = 0x09,
	MMM01 = 0x0b,
	MMM01_RAM = 0x0c,
	MMM01_RAM_BATTERY = 0x0d,
	MBC3_TIMER_BATTERY = 0x0f,
	MBC3_TIMER_RAM_BATTERY = 0x10,
	MBC3 = 0x11,
	MBC3_RAM = 0x12,
	MBC3_RAM_BATTERY = 0x13,
	MBC5 = 0x19,
	MBC5_RAM = 0x1a,
	MBC5_RAM_BATTERY = 0x1b,
	MBC5_RUMBLE = 0x1c,
	MBC5_RUMBLE_RAM = 0x1d,
	MBC5_RUMBLE_RAM_BATTERY = 0x1e,
	MBC6 = 0x20,
	MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
	POCKET_CAMERA = 0xfc,
	BANDAI_TAMA5 = 0xfd,
	HuC3 = 0xfe,
	HuC1_RAM_BATTERY = 0xff,
};

std::string MMCTypeToString(MMCType type);

class MMC {
public:
	static const uint32_t OAMSize = 0xA0;

	MMC(Bus& bus);

	
	void Reset();
	virtual void LoadROM(uint8_t* data, uint32_t romSize);
	void PrintROMInfo(uint8_t* data, uint32_t romSize);

	virtual uint8_t Read8(uint16_t address);
	virtual void Write8(uint16_t address, uint8_t value);
	virtual uint8_t ReadHRAM(uint16_t address);
	virtual void WriteHRAM(uint16_t address, uint8_t value);
	virtual uint8_t ReadOAM(uint16_t address);
	virtual void WriteOAM(uint16_t address, uint8_t value);
	virtual OAMEntry* GetOAMEntry(uint8_t index);

	uint8_t ReadVBK();
	uint8_t ReadSVBK();
	void WriteVBK(uint8_t value);
	void WriteSVBK(uint8_t value);

private:
	Bus& bus;

	uint8_t VBK;
	uint8_t SVBK;

	std::array<uint8_t, 0x4000> rom0;
	std::array<uint8_t, 0x4000> rom1;
	std::array<std::array<uint8_t, 0x2000>,2> vram;
	std::array<uint8_t, 0x2000> eram;
	std::array< std::array<uint8_t, 0x1000>, 8> wram;
	std::array<uint8_t, OAMSize> oam;
	std::array<uint8_t, 0x100> hram;

	friend class Bus;
	friend class PPU;
};

