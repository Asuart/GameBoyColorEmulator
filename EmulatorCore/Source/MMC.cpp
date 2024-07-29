#include "MMC.h"
#include <vector>

const std::vector<uint8_t> initialTileData = {
	0xF0, 0xF0, 0xFC, 0xFC, 0xFC, 0xFC, 0xF3, 0xF3,
	0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
	0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0xF3, 0xF3,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCF, 0xCF,
	0x00, 0x00, 0x0F, 0x0F, 0x3F, 0x3F, 0x0F, 0x0F,
	0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x0F, 0x0F,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF3, 0xF3,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFF,
	0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC3, 0xC3,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFC,
	0xF3, 0xF3, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0x3C, 0x3C, 0xFC, 0xFC, 0xFC, 0xFC, 0x3C, 0x3C,
	0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3,
	0xF3, 0xF3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3,
	0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF,
	0x3C, 0x3C, 0x3F, 0x3F, 0x3C, 0x3C, 0x0F, 0x0F,
	0x3C, 0x3C, 0xFC, 0xFC, 0x00, 0x00, 0xFC, 0xFC,
	0xFC, 0xFC, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF0, 0xF0,
	0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF,
	0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xC3, 0xC3,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xFC, 0xFC,
	0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C
};

const std::vector<uint8_t> initialTileMapData = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x09, 0x0A, 0x0B, 0x0C, 0x19, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
	0x15, 0x16, 0x17, 0x18
};

std::string MMCTypeToString(MMCType type) {
	switch (type) {
	case MMCType::ROM_ONLY:
		return "ROM_ONLY";
	case MMCType::MBC1:
		return "MBC1";
	case MMCType::MBC1_RAM:
		return "MBC1_RAM";
	case MMCType::MBC1_RAM_BATTERY:
		return "MBC1_RAM_BATTERY";
	case MMCType::MBC2:
		return "MBC2";
	case MMCType::MBC2_BATTERY:
		return "MBC2_BATTERY";
	case MMCType::ROM_RAM:
		return "ROM_RAM";
	case MMCType::ROM_RAM_BATTERY:
		return "ROM_RAM_BATTERY";
	case MMCType::MMM01:
		return "MMM01";
	case MMCType::MMM01_RAM:
		return "MMM01_RAM";
	case MMCType::MMM01_RAM_BATTERY:
		return "MMM01_RAM_BATTERY";
	case MMCType::MBC3_TIMER_BATTERY:
		return "MBC3_TIMER_BATTERY";
	case MMCType::MBC3_TIMER_RAM_BATTERY:
		return "MBC3_TIMER_RAM_BATTERY";
	case MMCType::MBC3:
		return "MBC3";
	case MMCType::MBC3_RAM:
		return "MBC3_RAM";
	case MMCType::MBC3_RAM_BATTERY:
		return "MBC3_RAM_BATTERY";
	case MMCType::MBC5:
		return "MBC5";
	case MMCType::MBC5_RAM:
		return "MBC5_RAM";
	case MMCType::MBC5_RAM_BATTERY:
		return "MBC5_RAM_BATTERY";
	case MMCType::MBC5_RUMBLE:
		return "MBC5_RUMBLE";
	case MMCType::MBC5_RUMBLE_RAM:
		return "MBC5_RUMBLE_RAM";
	case MMCType::MBC5_RUMBLE_RAM_BATTERY:
		return "MBC5_RUMBLE_RAM_BATTERY";
	case MMCType::MBC6:
		return "MBC6";
	case MMCType::MBC7_SENSOR_RUMBLE_RAM_BATTERY:
		return "MBC7_SENSOR_RUMBLE_RAM_BATTERY";
	case MMCType::POCKET_CAMERA:
		return "POCKET_CAMERA";
	case MMCType::BANDAI_TAMA5:
		return "BANDAI_TAMA5";
	case MMCType::HuC3:
		return "HuC3";
	case MMCType::HuC1_RAM_BATTERY:
		return "HuC1_RAM_BATTERY";
	default:
		return "Undefined Memory Controller";
	}
}

MMC::MMC(Bus& bus) : bus(bus) {}

void MMC::Reset() {
	for (int32_t i = 0; i < hram.size(); i++) {
		hram[i] = 0xff;
	}
	for (int32_t i = 0; i < vram.size(); i++) {
		vram[0][i] = 0x00;
		vram[1][i] = 0x00;
	}
	for (int32_t i = 0; i < initialTileData.size(); i++) {
		vram[0][i * 2 + 0x10] = initialTileData[i];
		vram[1][i * 2 + 0x10] = initialTileData[i];
	}
	for (int32_t i = 0; i < initialTileMapData.size(); i++) {
		vram[0][i + 0x1904] = initialTileMapData[i];
		vram[1][i + 0x1904] = initialTileMapData[i];
	}
	VBK = 0;
	SVBK = 1;
}

void MMC::LoadROM(uint8_t* data, uint32_t romSize) {
	PrintROMInfo(data, romSize);
	for (uint32_t i = 0; i < romSize; i++) {
		Write8(i, data[i]);
	}
}

void MMC::PrintROMInfo(uint8_t* data, uint32_t romSize) {
	std::cout << "ROM Memory Controller: " << MMCTypeToString((MMCType)data[0x147]) << "\n";
	std::cout << "ROM Banks: " << (int32_t)(2 << data[0x148]) << "\n";
	std::cout << "RAM Size: " << (int32_t)data[0x149] << "\n";
	switch (data[0x149]) {
	case 0x00:
		std::cout << "RAM: No RAM\n";
		break;
	case 0x01:
		std::cout << "RAM: Unused\n";
		break;
	case 0x02:
		std::cout << "RAM: 8KiB\n";
		break;
	case 0x03:
		std::cout << "RAM: 32KiB\n";
		break;
	case 0x04:
		std::cout << "RAM: 128KiB\n";
		break;
	case 0x05:
		std::cout << "RAM: 64KiB\n";
		break;
	}
}

uint8_t MMC::Read8(uint16_t address) {
	if (address < 0x4000) {
		return rom0[address];
	}
	else if (address < 0x8000) {
		return rom1[address - 0x4000];
	}
	else if (address < 0xa000) {
		return vram[VBK & 1][address - 0x8000];
	}
	else if (address < 0xc000) {
		return eram[address - 0xa000];
	}
	else if (address < 0xd000) {
		return wram[0][address - 0xc000];
	}
	else if (address < 0xe000) {
		return wram[SVBK & 0b111][address - 0xd000];
	}
	return 0;
}

void MMC::Write8(uint16_t address, uint8_t value) {
	if (address < 0x4000) {
		rom0[address] = value;
	}
	else if (address < 0x8000) {
		rom1[address - 0x4000] = value;
	}
	else if (address < 0xa000) {
		vram[VBK & 1][address - 0x8000] = value;
	}
	else if (address < 0xc000) {
		eram[address - 0xa000] = value;
	}
	else if (address < 0xd000) {
		wram[0][address - 0xc000] = value;
	}
	else if (address < 0xe000) {
		wram[SVBK & 0b111][address - 0xd000] = value;
	}
}

uint8_t MMC::ReadHRAM(uint16_t address) {
	return hram[address & 0xff];
}

void MMC::WriteHRAM(uint16_t address, uint8_t value) {
	hram[address & 0xff] = value;
}

uint8_t MMC::ReadOAM(uint16_t address) {
	return oam[address - 0xfe00];
}

void MMC::WriteOAM(uint16_t address, uint8_t value) {
	oam[address - 0xfe00] = value;
}

OAMEntry* MMC::GetOAMEntry(uint8_t index) {
	return (OAMEntry*)&oam[(index % 40) * 4];
}

uint8_t MMC::ReadVBK() {
	return VBK;
}

uint8_t MMC::ReadSVBK() {
	return SVBK;
}

void MMC::WriteVBK(uint8_t value) {
	VBK = value | 0xfe;
}

void MMC::WriteSVBK(uint8_t value) {
	SVBK = value;
	if ((SVBK & 0b111) == 0) SVBK |= 1;
}