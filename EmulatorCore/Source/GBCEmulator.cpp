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
