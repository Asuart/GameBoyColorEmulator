#include "DMA.h"

DMA::DMA(Bus& bus) : bus(bus) {}

void DMA::Reset() {
	HDMA1 = 0xff;
	HDMA2 = 0xff;
	HDMA3 = 0xff;
	HDMA4 = 0xff;
	HDMA5 = 0xff;
	active = false;
	writes = 0;
}

void DMA::Step(uint32_t cpuCycles) {
	if (!active) return;
	for (uint32_t i = 0; i < cpuCycles && writes < 0xa0; i++, writes++) {
		bus.Write8(0xfe00 | writes, bus.Read8((((uint16_t)OAM) << 8) | writes, true), true);
	}
	if (writes >= 0xa0) active = false;
}

uint8_t DMA::ReadOAM() {
	return OAM;
}

uint8_t DMA::ReadHDMA1() {
	return HDMA1;
}

uint8_t DMA::ReadHDMA2() {
	return HDMA2;
}

uint8_t DMA::ReadHDMA3() {
	return HDMA3;
}

uint8_t DMA::ReadHDMA4() {
	return HDMA4;
}

uint8_t DMA::ReadHDMA5() {
	return HDMA5;
}

void DMA::WriteOAM(uint8_t value) {
	OAM = value % 0xe0;
	writes = 0;
	active = true;
}

void DMA::WriteHDMA1(uint8_t value) {
	HDMA1 = value;
}

void DMA::WriteHDMA2(uint8_t value) {
	HDMA2 = value;
}

void DMA::WriteHDMA3(uint8_t value) {
	HDMA3 = value;
}

void DMA::WriteHDMA4(uint8_t value) {
	HDMA4 = value;
}

void DMA::WriteHDMA5(uint8_t value) {
	HDMA5 = value;
}