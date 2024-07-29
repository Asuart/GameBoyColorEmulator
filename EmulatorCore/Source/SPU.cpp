#include "SPU.h"

std::array<uint16_t, 4> dutyCycles = {
	0b11111110'11111110,
	0b01111110'01111110,
	0b01111000'01111000,
	0b10000001'10000001
};

SPU::SPU(Bus& bus) : bus(bus) {}

void SPU::Reset() {
	clockAccumulator = 0;
	sampleCounter = 0;
}

void SPU::ClearSamples() {
	uint32_t sampleAlign = sampleCounter % 4;
	if (sampleAlign) {
		for (uint32_t i = 0; i < sampleAlign; i++) {
			//samples[i] = samples[sampleCounter - sampleAlign + i];
		}
	}
	sampleCounter = sampleAlign;
}

uint32_t SPU::GetSamplesCount() {
	return sampleCounter;
}

int16_t* SPU::GetSamples() {
	return samples.data();
}

void SPU::Step(uint32_t cpuClocks) {
	//if (!audioOn) return;
	const int16_t scale = SHRT_MAX / 4;
	clockAccumulator += cpuClocks / 4;
	
	while (clockAccumulator >= (spuSampleClock)) {
		pulse1.Step();
		uint16_t sample = pulse1.Sample();
		samples[sampleCounter % samples.size()] = scale * sample / 8;
		clockAccumulator -= spuSampleClock;
		sampleCounter++;
	}
}

void SPU::Tick() {
	//tickCounter++;
	//if ((tickCounter % 2) == 0) TickSoundLength();
	//if ((tickCounter % 4) == 0) TickFrequencySweep();
	//if ((tickCounter % 8) == 0) TickEnvelopeSweep();
}

void SPU::TickSoundLength() {
	if (pulse1.active) {
		pulse1.initialLengthTimer++;
		if (pulse1.initialLengthTimer == 0) pulse1.active = false;
	}
}

void SPU::TickFrequencySweep() {
	//if (pulse1.active) {
		//pulse1.waveCycle = (pulse1.waveCycle + 1) % 16;
	//}
}

void SPU::TickEnvelopeSweep() {

}

uint8_t SPU::ReadNR10() {
	return 0x00;
}

uint8_t SPU::ReadNR11() {
	return pulse1.waveDuty << 6;;
}

uint8_t SPU::ReadNR12() {
	return pulse1.volumeAndEnvelope;
}

uint8_t SPU::ReadNR13() {
	return 0x00;
}

uint8_t SPU::ReadNR14() {
	return pulse1.lengthEnabled << 6;
}

uint8_t SPU::ReadNR21() {
	return pulse2.lengthAndDuty;
}

uint8_t SPU::ReadNR22() {
	return pulse2.volumeAndEnvelope;
}

uint8_t SPU::ReadNR23() {
	return pulse2.periodLow;
}

uint8_t SPU::ReadNR24() {
	return pulse2.periodHighAndControl;
}

uint8_t SPU::ReadNR30() {
	return wave.dacEnable;
}

uint8_t SPU::ReadNR31() {
	return wave.lengthTimer;
}

uint8_t SPU::ReadNR32() {
	return wave.outputLevel;
}

uint8_t SPU::ReadNR33() {
	return wave.periodLow;
}

uint8_t SPU::ReadNR34() {
	return wave.periodHighAndControl;
}

uint8_t SPU::ReadNR41() {
	return noise.lengthTimer;
}

uint8_t SPU::ReadNR42() {
	return noise.volumeAndEnvelope;
}

uint8_t SPU::ReadNR43() {
	return noise.frequencyAndRandomness;
}

uint8_t SPU::ReadNR44() {
	return noise.control;
}

uint8_t SPU::ReadNR50() {
	return masterVolumeAndVIN;
}

uint8_t SPU::ReadNR51() {
	return soundPanning;
}

uint8_t SPU::ReadNR52() {
	ch1On = pulse1.active;
	return audioMasterControl;
}

uint8_t SPU::ReadWavePattern(uint16_t address) {
	return wave.pattern[address & 0xf];
}

uint8_t SPU::ReadPCM12() {
	return 0x00;
}

uint8_t SPU::ReadPCM34() {
	return 0x00;
}

void SPU::WriteNR10(uint8_t value) {
	return;
}

void SPU::WriteNR11(uint8_t value) {
	pulse1.lengthAndDuty = value;
	pulse1.lengthTimer = 64 - pulse1.initialLengthTimer;
}

void SPU::WriteNR12(uint8_t value) {
	pulse1.volumeAndEnvelope = value;
	if (pulse1.initialVolume == 0 && pulse1.envelopeDirection == 0) {
		pulse1.active = false;
	}
}

void SPU::WriteNR13(uint8_t value) {
	pulse1.periodLowBuffer = value;
	pulse1.soundPeriod = (pulse1.soundPeriod & 0x700) + value;
	pulse1.framePeriod = 4 * (0x800 - pulse1.soundPeriod);
}

void SPU::WriteNR14(uint8_t value) {
	pulse1.periodHighBuffer = value & 0x7;
	pulse1.trigger = value & 0x80;
	pulse1.lengthEnabled = value & 0x40;
	pulse1.soundPeriod = ((value << 8) & 0x0700) + (pulse1.soundPeriod & 0xff);
	pulse1.framePeriod = 4 * (0x800 - pulse1.soundPeriod);
	if (pulse1.trigger) pulse1.Trigger();
}

void SPU::WriteNR21(uint8_t value) {
	pulse2.lengthAndDuty = value;
	pulse2.lengthTimer = 64 - pulse2.initialLengthTimer;
}

void SPU::WriteNR22(uint8_t value) {
	pulse2.volumeAndEnvelope = value;
	if (pulse2.initialVolume == 0 && pulse2.envelopeDirection == 0) pulse2.active = false;
}

void SPU::WriteNR23(uint8_t value) {
	pulse2.periodLowBuffer = value;
}

void SPU::WriteNR24(uint8_t value) {
	pulse2.periodHighBuffer = value & 0x7;
	pulse2.trigger = value & 0x80;
	pulse2.lengthEnabled = value & 0x40;
	if (pulse2.trigger) pulse2.Trigger();
}

void SPU::WriteNR30(uint8_t value) {
	wave.dacEnable = value;
}

void SPU::WriteNR31(uint8_t value) {
	wave.lengthTimer = value;
}

void SPU::WriteNR32(uint8_t value) {
	wave.outputLevel = value;
}

void SPU::WriteNR33(uint8_t value) {
	wave.periodLowBuffer = value;
}

void SPU::WriteNR34(uint8_t value) {
	wave.periodHighBuffer = value & 0x7;
	wave.trigger = value & 0x80;
	wave.lengthEnable = value & 0x40;
	if (wave.trigger) wave.Trigger();
}

void SPU::WriteNR41(uint8_t value) {
	noise.lengthTimer = value;
}

void SPU::WriteNR42(uint8_t value) {
	noise.volumeAndEnvelope = value;
}

void SPU::WriteNR43(uint8_t value) {
	noise.frequencyAndRandomness = value;
}

void SPU::WriteNR44(uint8_t value) {
	noise.control = value;
	if (noise.trigger) noise.Trigger();
}

void SPU::WriteNR50(uint8_t value) {
	masterVolumeAndVIN = value;
}

void SPU::WriteNR51(uint8_t value) {
	soundPanning = value;
}

void SPU::WriteNR52(uint8_t value) {
	audioOn = value & 0x80;
}

void SPU::WriteWavePattern(uint16_t address, uint8_t value) {
	wave.pattern[address & 0xf] = value;
}

SPU::PulseChannel::PulseChannel() {
	initialVolume = 0;
	envelopeDirection = 0;
	sweepPace = 0;
	soundPeriod = 0;
	lengthEnabled = 0;
	activePace = 0x00;
	lengthAndDuty = 0x00;
	volumeAndEnvelope = 0x00;
	periodLow = 0x00;
	periodHighAndControl = 0x00;
	periodLowBuffer = 0x00;
	periodHighBuffer = 0x00;
	waveCycle = 0x00;
	active = false;
	lengthTimer = 64;
	periodTimer = 0;
	envelopeTimer = 0;
	framePeriod = 4;
	waveDuty = 0;
	frameTimer = 0x2000;
	frame = 0;
	volume = 0;
}

void SPU::PulseChannel::Step() {
	periodTimer -= spuSampleClock;
	while (periodTimer <= 0) {
		periodTimer += framePeriod;
		waveCycle = (waveCycle + 1) % 16;
	}

	frameTimer -= spuSampleClock;
	while (frameTimer <= 0) {
		frameTimer += 0x2000;
		TickFrame();
	}
}

void SPU::PulseChannel::TickFrame() {
	frame = (frame + 1) % 8;
	if (lengthEnabled && (frame & 1) == 0 && lengthTimer > 0) {
		lengthTimer--;
		if (lengthTimer == 0) {
			active = false;
		}
	}
	if (frame == 7 && envelopeTimer != 0) {
		envelopeTimer -= 1;
		if (envelopeTimer == 0) {
			int32_t newVolume = volume + envelopeDirection ? 1 : -1;
			if (!(newVolume < 0 || newVolume > 15)) {
				envelopeTimer = sweepPace;
				volume = newVolume;
			}
		}
	}
}

int16_t SPU::PulseChannel::Sample() {
	return (volume + 1) * ((dutyCycles[waveDuty] >> waveCycle) & 1);
}

void SPU::PulseChannel::Trigger() {
	active = !(sweepPace == 0 && initialVolume == 0);
	lengthTimer = lengthTimer ? lengthTimer : 64;
	periodTimer = period;
	envelopeTimer = sweepPace;
	volume = initialVolume;
}

void SPU::WaveChannel::Trigger() {

}

void SPU::NoiseChannel::Trigger() {

}