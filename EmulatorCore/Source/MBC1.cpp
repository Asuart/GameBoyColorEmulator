#include "MBC1.h"

MBC1::MBC1(Bus& bus) : MMC(bus) {}

void MBC1::Reset() {
	RAMEnable = 0;
	ROMBank = 0;
	RAMBank = 0;
	mode = 0;
}

void MBC1::LoadROM(uint8_t* data, uint32_t romSize) {
	PrintROMInfo(data, romSize);
	ROMBanksCount = (2 << data[0x148]);
	switch (data[0x149]) {
	case 0x00:
		RAMBanksCount = 1;
		break;
	case 0x01:
		RAMBanksCount = 1;
		break;
	case 0x02:
		RAMBanksCount = 1;
		break;
	case 0x03:
		RAMBanksCount = 4;
		break;
	case 0x04:
		RAMBanksCount = 4;
		std::cout << "Error: RAM size is to big for mapper.\n";
		break;
	case 0x05:
		RAMBanksCount = 4;
		std::cout << "Error: RAM size is to big for mapper.\n";
		break;
	}
	for (int32_t i = 0; i < ROMBanksCount && i < romBanks.size(); i++) {
		for (int32_t j = 0; j < 0x4000; j++) {
			romBanks[i][j] = data[i * 0x4000 + j];
		}
	}
}

uint8_t MBC1::Read8(uint16_t address) {
	if (address < 0x4000) {
		return romBanks[GetROMBank0()][address];
	}
	else if (address < 0x8000) {
		return romBanks[GetROMBank()][address - 0x4000];
	}
	else if (address < 0xa000) {
		return vram[address - 0x8000];
	}
	else if (address < 0xc000) {
		if (!RAMEnable) {
			std::cout << "Read from disabled RAM.\n";
			return 0xff;
		}
		return ramBanks[GetRAMBank()][address - 0xa000];
	}
	else if (address < 0xd000) {
		return wram0[address - 0xc000];
	}
	else if (address < 0xe000) {
		return wram1[address - 0xd000];
	}
	return 0;
}

void MBC1::Write8(uint16_t address, uint8_t value) {
	if (address < 0x8000) {
		if (address < 0x2000) {
			RAMEnable = (value & 0x0f) == 0x0a;
		}
		else if (address < 0x4000) {
			ROMBank = value & 0x1f;
			if (ROMBank == 0) ROMBank = 1;
		}
		else if (address < 0x6000) {
			RAMBank = value & 0x03;
		}
		else {
			mode = value & 1;
		}
	}
	else if (address < 0xa000) {
		vram[address - 0x8000] = value;
	}
	else if (address < 0xc000) {
		if (!RAMEnable) {
			std::cout << "Write ti disabled RAM\n";
			return;
		}
		ramBanks[GetRAMBank()][address - 0xa000] = value;
	}
	else if (address < 0xd000) {
		wram0[address - 0xc000] = value;
	}
	else if (address < 0xe000) {
		wram1[address - 0xd000] = value;
	}
}

uint8_t MBC1::ReadHRAM(uint16_t address) {
	return hram[address & 0xff];
}

void MBC1::WriteHRAM(uint16_t address, uint8_t value) {
	hram[address & 0xff] = value;
}

uint8_t MBC1::ReadOAM(uint16_t address) {
	return oam[address - 0xfe00];
}

void MBC1::WriteOAM(uint16_t address, uint8_t value) {
	oam[address - 0xfe00] = value;
}

OAMEntry* MBC1::GetOAMEntry(uint8_t index) {
	return(OAMEntry*)&oam[(index % 40) * 4];
}

uint8_t MBC1::GetROMBank0() {
	if (mode && ROMBanksCount > 32) {
		return RAMBank << 5;
	}
	return 0;
}

uint8_t MBC1::GetROMBank() {
	if (ROMBanksCount > 32) {
		return (RAMBank << 5) | ROMBank;
	}
	if (ROMBank == 0) return 1;
	return ROMBank & (ROMBanksCount - 1);
}

uint8_t MBC1::GetRAMBank() {
	if (mode) {
		return RAMBank & 0b11;
	}
	return 0;
}
