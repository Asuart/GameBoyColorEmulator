#include "GBCEmulator.h"

GBCEmulator::GBCEmulator() : mmc(new MMC(bus)), dma(bus), joypad1(bus), timer(bus), spu(bus), ppu(bus), cpu(bus),
bus(mmc, dma, joypad1, timer, spu, ppu, cpu) {}

void GBCEmulator::Reset() {
	bus.Reset();
	clockAligner = 0;
}

bool GBCEmulator::LoadROM(uint8_t* data, uint32_t romSize) {
	delete mmc;
	mmc = CreateMMC((MMCType)data[0x147]);
	bus.mmc = mmc;
	Reset();
	mmc->LoadROM(data, romSize);
	romLoaded = true;
	return true;
}

void GBCEmulator::Run(uint32_t cpuCycles) {
	if (!romLoaded) return;
	clockAligner += cpuCycles;
	while (clockAligner > 0) {
		uint32_t cycles = cpu.Step();
		clockAligner -= cycles;
		timer.Step(cycles);
		dma.Step(cycles);
		//spu.Step(cycles);
		ppu.StepScanlineMode(cycles);
	}
}

bool GBCEmulator::IsFrameReady() {
	return ppu.frameReady;
}

void GBCEmulator::ResetFrameReadyFlag() {
	ppu.frameReady = false;
}

SaveState GBCEmulator::GetSaveState() {
	SaveState state("test");



	uint32_t cursor = 0;
	uint8_t* saveState = new uint8_t[saveStateSize];

	auto write8 = [&](uint8_t value) {
		saveState[cursor++] = value;
		};

	auto write16 = [&](uint16_t value) {
		saveState[cursor++] = value & 0xff;
		saveState[cursor++] = value >> 8;
		};

	auto write32 = [&](uint32_t value) {
		uint32_t* ptr = (uint32_t*)&saveState[cursor];
		ptr[cursor] = value;
		cursor += 4;
		};

	auto write64 = [&](uint64_t value) {
		uint64_t* ptr = (uint64_t*)&saveState[cursor];
		ptr[cursor] = value;
		cursor += 8;
		};

	// bus
	write8(bus.IF);
	write8(bus.IE);
	write8(bus.key1);

	// cpu
	write16(cpu.AF);
	write16(cpu.BC);
	write16(cpu.DE);
	write16(cpu.HL);
	write16(cpu.SP);
	write16(cpu.SP);
	write8(cpu.IME);
	write8(cpu.isHalting);
	write8(cpu.haltBug);

	// dma
	write8(dma.OAM);
	write8(dma.HDMA1);
	write8(dma.HDMA2);
	write8(dma.HDMA3);
	write8(dma.HDMA4);
	write8(dma.HDMA5);

	// joypad
	write8(joypad1.buttons[0]);
	write8(joypad1.buttons[1]);
	write8(joypad1.buttons[2]);
	write8(joypad1.buttons[3]);
	write8(joypad1.buttons[4]);
	write8(joypad1.buttons[5]);
	write8(joypad1.buttons[6]);
	write8(joypad1.buttons[7]);
	write8(joypad1.data);

	return state;
}

bool GBCEmulator::LoadSaveState(const SaveState& saveState) {

}

MMC* GBCEmulator::CreateMMC(MMCType type) {
	switch (type) {
	case MMCType::MBC1:
	case MMCType::MBC1_RAM:
	case MMCType::MBC1_RAM_BATTERY:
		return new MBC1(bus);
	default:
		std::cout << "Unhandled Memory Controller: " << MMCTypeToString(type) << ". Default created instead.\n;";
		return new MMC(bus);
	case MMCType::ROM_ONLY:
		return new MMC(bus);
	}
}
