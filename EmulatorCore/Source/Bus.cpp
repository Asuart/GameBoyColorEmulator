#include "Bus.h"

Bus::Bus(MMC* mmc, DMA& dma, Joypad& joypad1, Timer& timer, SPU& spu, PPU& ppu, CPU& cpu)
	: mmc(mmc), dma(dma), joypad1(joypad1), timer(timer), spu(spu), ppu(ppu), cpu(cpu) {}

void Bus::Reset() {
	mmc->Reset();
	dma.Reset();
	joypad1.Reset();
	timer.Reset();
	spu.Reset();
	ppu.Reset();
	cpu.Reset();
}

void Bus::TriggerInterruption(Interruption interruption) {
	cpu.IF |= (uint8_t)interruption;
}

uint8_t Bus::Read8(uint16_t  address, bool isDMAAccess) {
	if (dma.active && address < 0xff80 && !isDMAAccess) {
		return 0xff;
	}
	if (address < 0xfe00) {
		if (ppu.LCDEnable && address >= 0x8000 && address < 0xa000 && ppu.mode == 3) {
			//return 0xff;
		}
		if (address >= 0xe000) address -= 0x2000;
		return mmc->Read8(address);
	}
	if (address < 0xfea0) {
		//if (ppu.LCDEnable && (ppu.PPUMode == 2 || ppu.PPUMode == 3)) return 0xff;
		return mmc->ReadOAM(address);
	}
	if (address >= 0xff00) {
		switch (address & 0xff) {
		case 0x00:
			return joypad1.Read();
		case 0x01: // Serial transfer data
		case 0x02: // Serial transfer control
			return mmc->ReadHRAM(address);
			return 0;
		case 0x04:
			return timer.ReadDIV();
		case 0x05:
			return timer.ReadTIMA();
		case 0x06:
			return timer.ReadTMA();
		case 0x07:
			return timer.ReadTAC();
		case 0x0f:
			return cpu.IF;
		case 0x10:
			return spu.ReadNR10();
		case 0x11:
			return spu.ReadNR11();
		case 0x12:
			return spu.ReadNR12();
		case 0x13:
			return spu.ReadNR13();
		case 0x14:
			return spu.ReadNR14();
		case 0x16:
			return spu.ReadNR21();
		case 0x17:
			return spu.ReadNR22();
		case 0x18:
			return spu.ReadNR23();
		case 0x19:
			return spu.ReadNR24();
		case 0x1a:
			return spu.ReadNR30();
		case 0x1b:
			return spu.ReadNR11();
		case 0x1c:
			return spu.ReadNR32();
		case 0x1d:
			return spu.ReadNR33();
		case 0x1e:
			return spu.ReadNR34();
		case 0x20:
			return spu.ReadNR41();
		case 0x21:
			return spu.ReadNR42();
		case 0x22:
			return spu.ReadNR43();
		case 0x23:
			return spu.ReadNR44();
		case 0x24:
			return spu.ReadNR50();
		case 0x25:
			return spu.ReadNR51();
		case 0x26:
			return spu.ReadNR52();
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			return spu.ReadWavePattern(address);
		case 0x40:
			return ppu.ReadLCDC();
		case 0x41:
			return ppu.ReadSTAT();
		case 0x42:
			return ppu.ReadSCY();
		case 0x43:
			return ppu.ReadSCX();
		case 0x44:
			return ppu.ReadLY();
		case 0x45:
			return ppu.ReadLYC();
		case 0x46:
			return 0x00;
		case 0x47:
			return ppu.ReadBGP();
		case 0x48:
			return ppu.ReadOBP0();
		case 0x49:
			return ppu.ReadOBP1();
		case 0x4a:
			return ppu.ReadWY();
		case 0x4b:
			return ppu.ReadWX();
		case 0x4d:
			return cpu.key1;
		case 0x4f:
			return mmc->ReadVBK();
		case 0x51:
			return 0x00;
		case 0x52:
			return 0x00;
		case 0x53:
			return 0x00;
		case 0x54:
			return 0x00;
		case 0x55:
			return dma.ReadHDMA5();
		case 0x56: // Infrared communications port
			return 0xff;
		case 0x68:
			return ppu.ReadBGPI();
		case 0x69:
			return ppu.ReadBGPD();
		case 0x6a:
			return ppu.ReadOBPI();
		case 0x6b:
			return ppu.ReadOBPD();
		case 0x6c:
			return ppu.ReadOPRI();
		case 0x70:
			return mmc->ReadSVBK();
		case 0x76:
			return spu.ReadPCM12();
		case 0x77:
			return spu.ReadPCM34();
		case 0xff:
			return cpu.IE;
		default:
			return mmc->ReadHRAM(address);
		}
	}
	std::cout << "Unhandled read from address: 0x" << std::hex << address << "\n";
	return 0;
}

void Bus::Write8(uint16_t address, uint8_t value, bool isDMAAccess) {
	if (dma.active && address < 0xff80 && !isDMAAccess) {
		return;
	}
	if (address < 0xfe00) {
		if (ppu.LCDEnable && address >= 0x8000 && address < 0xa000 && ppu.mode == 3) {
			//return;
		}
		if (address >= 0xe000) address -= 0x2000;
		mmc->Write8(address, value);
		return;
	}
	if (address < 0xfea0) {
		//if (ppu.LCDEnable && (ppu.mode == 2 || ppu.mode == 3)) return;
		mmc->WriteOAM(address, value);
		return;
	}
	if (address >= 0xff00) {
		switch (address & 0xff) {
		case 0x00:
			joypad1.Write(value);
			return;
		case 0x01: // Serial transfer data
			std::cout << "Write to serial: " << std::hex << (int32_t)value << "(" << value << ")" << "\n";
			mmc->WriteHRAM(address, value);
			return;
		case 0x02: // Serial transfer control
			//std::cout << "Write to serial control: " << std::hex << (int32_t)value << "\n";
			mmc->WriteHRAM(address, value);
			return;
		case 0x04:
			timer.WriteDIV(value);
			return;
		case 0x05:
			timer.WriteTIMA(value);
			return;
		case 0x06:
			timer.WriteTMA(value);
			return;
		case 0x07:
			timer.WriteTAC(value);
			return;
		case 0x0f:
			cpu.IF = value;
			return;
		case 0x10:
			spu.WriteNR10(value);
			return;
		case 0x11:
			spu.WriteNR11(value);
			return;
		case 0x12:
			spu.WriteNR12(value);
			return;
		case 0x13:
			spu.WriteNR13(value);
			return;
		case 0x14:
			spu.WriteNR14(value);
			return;
		case 0x16:
			spu.WriteNR21(value);
			return;
		case 0x17:
			spu.WriteNR22(value);
			return;
		case 0x18:
			spu.WriteNR23(value);
			return;
		case 0x19:
			spu.WriteNR24(value);
			return;
		case 0x1a:
			spu.WriteNR30(value);
			return;
		case 0x1b:
			spu.WriteNR11(value);
			return;
		case 0x1c:
			spu.WriteNR32(value);
			return;
		case 0x1d:
			spu.WriteNR33(value);
			return;
		case 0x1e:
			spu.WriteNR34(value);
			return;
		case 0x20:
			spu.WriteNR41(value);
			return;
		case 0x21:
			spu.WriteNR42(value);
			return;
		case 0x22:
			spu.WriteNR43(value);
			return;
		case 0x23:
			spu.WriteNR44(value);
			return;
		case 0x24:
			spu.WriteNR50(value);
			return;
		case 0x25:
			spu.WriteNR51(value);
			return;
		case 0x26:
			spu.WriteNR52(value);
			return;
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			spu.WriteWavePattern(address, value);
			return;
		case 0x40:
			ppu.WriteLCDC(value);
			return;
		case 0x41:
			ppu.WriteSTAT(value);
			return;
		case 0x42:
			ppu.WriteSCY(value);
			return;
		case 0x43:
			ppu.WriteSCX(value);
			return;
		case 0x44:
			ppu.WriteLY(value);
			return;
		case 0x45:
			ppu.WriteLYC(value);
			return;
		case 0x46:
			dma.WriteOAM(value);
			return;
		case 0x47:
			ppu.WriteBGP(value);
			return;
		case 0x48:
			ppu.WriteOBP0(value);
			return;
		case 0x49:
			ppu.WriteOBP1(value);
			return;
		case 0x4a:
			ppu.WriteWY(value);
			return;
		case 0x4b:
			ppu.WriteWX(value);
			return;
		case 0x4d:
			cpu.key1 = value;
			return;
		case 0x4f:
			mmc->WriteVBK(value);
			return;
		case 0x51:
			dma.WriteHDMA1(value);
			return;
		case 0x52:
			dma.WriteHDMA2(value);
			return;
		case 0x53:
			dma.WriteHDMA3(value);
			return;
		case 0x54:
			dma.WriteHDMA4(value);
			return;
		case 0x55:
			dma.WriteHDMA5(value);
			return;
		case 0x56: // Infrared communications port
			mmc->WriteHRAM(address, value);
			return;
		case 0x68:
			ppu.WriteBGPI(value);
			return;
		case 0x69:
			ppu.WriteBGPD(value);
			return;
		case 0x6a:
			ppu.WriteOBPI(value);
			return;
		case 0x6b:
			ppu.WriteOBPD(value);
			return;
		case 0x6c:
			ppu.WriteOPRI(value);
			return;
		case 0x70:
			mmc->WriteSVBK(value);
			return;
		case 0xff:
			cpu.IE = value;
			return;
		default:
			if ((address & 0xff) >= 0x80) mmc->WriteHRAM(address, value);
			return;
		}
	}
	std::cout << "Unhandled write to address: 0x" << std::hex << address << "\n";
}

uint16_t Bus::Read16(uint16_t address) {
	return (uint16_t)Read8(address) | ((uint16_t)Read8(address + 1) << 8);
}

void Bus::Write16(uint16_t address, uint16_t value) {
	Write8(address, value & 0xff);
	Write8(address + 1, value >> 8);
}

OAMEntry* Bus::GetOAMEntry(uint8_t index) {
	return mmc->GetOAMEntry(index);
}

void Bus::TriggerSPUTick() {
	spu.Tick();
}