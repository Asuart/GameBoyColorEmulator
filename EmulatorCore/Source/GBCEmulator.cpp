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

SaveState* GBCEmulator::GetSaveState() {
	return saveState;
}

SaveState* GBCEmulator::CreateSaveState() {
	if (saveState) delete saveState;
	saveState = new SaveState(romName);
	cpu.WriteState(*saveState);
	dma.WriteState(*saveState);
	joypad1.WriteState(*saveState);
	mmc->WriteState(*saveState);
	ppu.WriteState(*saveState);
	timer.WriteState(*saveState);
	//spu.WriteState(*saveState);
	return saveState;
}

void GBCEmulator::SetSaveState(SaveState* state) {
	if (saveState) delete saveState;
	saveState = state;
}

bool GBCEmulator::LoadSaveState() {
	if (!saveState) return false;
	try {
		cpu.LoadState(*saveState);
		dma.LoadState(*saveState);
		joypad1.LoadState(*saveState);
		mmc->LoadState(*saveState);
		ppu.LoadState(*saveState);
		timer.LoadState(*saveState);
		//spu.LoadState(*saveState);
	}
	catch (std::exception e) {
		std::cout << "Failed to load save state: " << e.what() << "\n";
		return false;
	}
	return true;
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

std::string GBCEmulator::GetROMName() {
	return romName;
}

void GBCEmulator::SetName(const std::string& name) {
	romName = name;
}