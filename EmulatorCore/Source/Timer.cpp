#include "Timer.h"

std::array<uint32_t, 4> timaDividers = { 1024, 16, 64, 256 };

Timer::Timer(Bus& bus) : bus(bus) {}

void Timer::Reset() {
	DIV = 0xac;
	TIMA = 0x00;
	TMA = 0x00;
	TAC = 0xF8;
	DIVClockAccumulator = 0;
	TIMAClockAccumulator = 0;
}

void Timer::Step(uint32_t cycles) {
	DIVClockAccumulator += cycles;
	uint8_t initialDIV = DIV;
	DIV += DIVClockAccumulator >> 8;
	DIVClockAccumulator &= 0xff;
	if (((initialDIV >> 4) & 1) && !((DIV >> 4) & 1)) bus.TriggerSPUTick();

	if ((TAC & 0b100) == 0) return;
	TIMAClockAccumulator += cycles;
	uint32_t divider = timaDividers[TAC & 0b11];
	while (TIMAClockAccumulator >= divider) {
		TIMAClockAccumulator -= divider;
		TIMA++;
		if (TIMA == 0x00) {
			TIMA = TMA;
			bus.TriggerInterruption(Interruption::Timer);
		}
	}
}

uint8_t Timer::ReadDIV() {
	return DIV;
}

uint8_t Timer::ReadTIMA() {
	return TIMA;
}

uint8_t Timer::ReadTMA() {
	return TMA;
}

uint8_t Timer::ReadTAC() {
	return TAC;
}

void Timer::WriteDIV(uint8_t value) {
	if ((DIV >> 4) & 1) bus.TriggerSPUTick();
	DIV = 0;
	DIVClockAccumulator = 0;
	TIMAClockAccumulator = 0;
}

void Timer::WriteTIMA(uint8_t value) {
	TIMA = value;
}

void Timer::WriteTMA(uint8_t value) {
	TMA = value;
}

void Timer::WriteTAC(uint8_t value) {
	TAC = value;
}