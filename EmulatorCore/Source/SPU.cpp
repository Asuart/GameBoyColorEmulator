#include "SPU.h"

std::array<uint16_t, 4> dutyCycles = {
	0b11111110'11111110,
	0b01111110'01111110,
	0b01111000'01111000,
	0b10000001'10000001
};

SPU::SPU(Bus& bus) : bus(bus) {
	audioOn = true;
}

void SPU::Reset() {
	clockAccumulator = 0;
	sampleCounter = 0;
	for (uint32_t i = 0; i < 5; i++) {
		sweepChannel.WriteRegister(i, 0);
		toneChannel.WriteRegister(i, 0);
		waveChannel.WriteRegister(i, 0);
		noiseChannel.WriteRegister(i, 0);
	}
}

void SPU::ClearSamples() {
	uint32_t sampleAlign = sampleCounter % 8;
	if (sampleAlign) {
		for (uint32_t i = 0; i < sampleAlign; i++) {
			samples[i] = samples[sampleCounter - sampleAlign + i];
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
	const int16_t scale = SHRT_MAX / 4;
	clockAccumulator += cpuClocks;

	while (clockAccumulator >= spuSampleClock) {
		if (true || audioOn) {
			toneChannel.Step();
			sweepChannel.Step();
			waveChannel.Step();
			noiseChannel.Step();
			int16_t sweepSample = sweepChannel.Sample();
			int16_t toneSample = toneChannel.Sample();
			int16_t waveSample = waveChannel.Sample();
			int16_t noiseSample = noiseChannel.Sample();
			int16_t leftSample = sweepSample * ch1Left + toneSample * ch2Left + waveSample * ch3Left + noiseSample * ch4Left;
			int16_t rightSample = sweepSample * ch1Right + toneSample * ch2Right + waveSample * ch3Right + noiseSample * ch4Right;
			samples[sampleCounter % samples.size()] = scale * leftSample / 32;
			samples[(sampleCounter + 1) % samples.size()] = scale * rightSample / 32;
		}
		else {
			samples[sampleCounter % samples.size()] = 0;
		}
		clockAccumulator -= spuSampleClock;
		sampleCounter += 2;
	}
}

void SPU::WriteState(SaveState& state) {
	// TODO
}

void SPU::LoadState(uint8_t* state) {
	// TODO
}

uint8_t SPU::ReadNR50() {
	return masterVolumeAndVIN;
}

uint8_t SPU::ReadNR51() {
	return soundPanning;
}

uint8_t SPU::ReadNR52() {
	ch1On = sweepChannel.enable;
	ch2On = toneChannel.enable;
	ch3On = waveChannel.enable;
	ch4On = noiseChannel.enable;
	return audioMasterControl;
}

uint8_t SPU::ReadPCM12() {
	return 0x00;
}

uint8_t SPU::ReadPCM34() {
	return 0x00;
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

uint8_t SPU::ToneChannel::ReadRegister(uint8_t reg) {
	switch (reg) {
	case 0:
		return 0;
	case 1:
		return wavsel << 6;
	case 2:
		return envini << 4 | envdir << 3 | envper;
	case 3:
		return 0;
	case 4:
		return uselen << 6;
	default:
		std::cout << "Read from out of range tone or sweep channel register.\n";
		return 0xff;
	}
}

void SPU::ToneChannel::WriteRegister(uint8_t reg, uint8_t value) {
	switch (reg) {
	case 0: break;
	case 1:
		wavsel = value >> 6;
		sndlen = value & 0x1f;
		lengthtimer = 64 - sndlen;
		break;
	case 2:
		envini = value >> 4;
		envdir = (value >> 3) & 0x1;
		envper = value & 0x7;
		if (envini == 0 && envdir == 0) {
			enable = false;
		}
		break;
	case 3:
		sndper = (sndper & 0x700) + value;
		period = 4 * (0x800 - sndper);
		break;
	case 4:
		uselen = (value >> 6) & 1;
		sndper = ((value << 8) & 0x700) + (sndper & 0xff);
		period = 4 * (0x800 - sndper);
		if (value & 0x80) {
			Trigger();
		}
		break;
	default:
		std::cout << "Write to out of range tone or seep channel register.\n";
	}
}

void SPU::ToneChannel::Step() {
	periodtimer -= spuSampleClock;
	while (periodtimer <= 0 && period) {
		periodtimer += period;
		waveframe = (waveframe + 1) % 8;
	}

	frametimer -= spuSampleClock;
	while (frametimer <= 0) {
		frametimer += 0x2000;
		TickFrame();
	}
}

void SPU::ToneChannel::TickFrame() {
	frame = (frame + 1) % 8;
	if (uselen && (frame & 1) == 0 && lengthtimer > 0) {
		lengthtimer--;
		if (lengthtimer == 0) {
			enable = false;
		}
	}
	if (frame == 7 && envelopetimer != 0) {
		envelopetimer--;
		if (envelopetimer == 0) {
			int32_t newVolume = volume + (envdir ? 1 : -1);
			if (!(newVolume < 0 || newVolume > 15)) {
				envelopetimer = envper;
				volume = newVolume;
			}
		}
	}
}

int16_t SPU::ToneChannel::Sample() {
	if (!enable) return 0;
	return (volume + 1) * ((dutyCycles[wavsel] >> waveframe) & 1);
}

void SPU::ToneChannel::Trigger() {
	enable = !(envper == 0 && envini == 0);
	lengthtimer = lengthtimer ? lengthtimer : 64;
	periodtimer = period;
	envelopetimer = envper;
	volume = envini;
}

uint8_t SPU::SweepChannel::ReadRegister(uint8_t reg) {
	if (reg == 0) {
		return swpper << 4 | swpdir << 3 | swpmag;
	}
	else {
		return SPU::ToneChannel::ReadRegister(reg);
	}
}

void SPU::SweepChannel::WriteRegister(uint8_t reg, uint8_t value) {
	if (reg == 0) {
		swpper = (value >> 4) & 0x7;
		swpdir = (value >> 3) & 0x1;
		swpmag = value & 0x7;
	}
	else {
		SPU::ToneChannel::WriteRegister(reg, value);
	}
}

void SPU::SweepChannel::TickFrame() {
	SPU::ToneChannel::TickFrame();
	if (sweepenable && swpper && (frame & 3) == 2) {
		sweeptimer--;
		if (sweeptimer == 0 && Sweep(true)) {
			sweeptimer = swpper;
			Sweep(false);
		}
	}
}

void SPU::SweepChannel::Trigger() {
	SPU::ToneChannel::Trigger();
	shadow = sndper;
	sweeptimer = swpper;
	sweepenable = swpper || swpmag;
	if (swpmag) {
		Sweep(false);
	}
}

bool SPU::SweepChannel::Sweep(bool save) {
	int16_t newper = 0;
	if (swpdir == 0) {
		newper = shadow + (shadow >> swpmag);
	}
	else {
		newper = shadow - (shadow >> swpmag);
	}
	if (newper >= 0x800) {
		enable = false;
		return false;
	}
	else {
		sndper = shadow = newper;
		period = 4 * (0x800 - sndper);
		return true;
	}
}

uint8_t SPU::WaveChannel::ReadRegister(uint8_t reg) {
	switch (reg) {
	case 0:
		return (dacpow << 7) | 0x7f;
	case 1:
		return 0xff;
	case 2:
		return (volreg << 5) | 0x9f;
	case 3:
		return 0xff;
	case 4:
		return (uselen << 6) | 0xbf;
	default:
		std::cout << "Read from out of range wave channel register.\n";
		return 0xff;
	}
}

void SPU::WaveChannel::WriteRegister(uint8_t reg, uint8_t value) {
	switch (reg) {
	case 0:
		dacpow = value >> 7;
		if (dacpow == 0) {
			enable = false;
		}
		break;
	case 1:
		sndlen = value;
		lengthtimer = 256 - sndlen;
		break;
	case 2:
		volreg = (value >> 5) & 0x3;
		volumeshift = (volreg > 0) ? (volreg - 1) : 4;
		break;
	case 3:
		sndper = (sndper & 0x700) + value;
		period = 2 * (0x800 - sndper);
		break;
	case 4:
		uselen = (value >> 6) & 0x1;
		sndper = ((value << 8) & 0x700) + (sndper & 0xff);
		period = 2 * (0x800 - sndper);
		if (value & 0x80) {
			Trigger();
		}
		break;
	default:
		std::cout << "Write to out of range wave channel register.\n";
	}
}

uint8_t SPU::WaveChannel::ReadWaveByte(uint8_t offset) {
	if (dacpow) {
		return wavetable[waveframe % 16];
	}
	else {
		return wavetable[offset % 16];
	}
}

void SPU::WaveChannel::WriteWaveByte(uint8_t offset, uint8_t value) {
	if (dacpow) {
		wavetable[waveframe % 16] = value;
	}
	else {
		wavetable[offset % 16] = value;
	}
}

void SPU::WaveChannel::Step() {
	periodtimer -= spuSampleClock;
	while (periodtimer <= 0 && period) {
		periodtimer += period;
		waveframe = (waveframe + 1) % 32;
	}

	frametimer -= spuSampleClock;
	while (frametimer <= 0) {
		frametimer += 0x2000;
		TickFrame();
	}
}

void SPU::WaveChannel::TickFrame() {
	frame = (frame + 1) % 8;
	if (uselen && (frame & 1) == 0 && lengthtimer > 0) {
		lengthtimer--;
		if (lengthtimer == 0) {
			enable = false;
		}
	}
}

int16_t SPU::WaveChannel::Sample() {
	if (enable && dacpow) {
		int16_t sample = (wavetable[waveframe / 2] >> (waveframe % 2 ? 0 : 4)) & 0xf;
		return sample >> volumeshift;
	}
	return 0;
}

void SPU::WaveChannel::Trigger() {
	enable = dacpow;
	lengthtimer = lengthtimer ? lengthtimer : 256;
	periodtimer = period;
}

uint8_t SPU::NoiseChannel::ReadRegister(uint8_t reg) {
	switch (reg) {
	case 0: case 1:
		return 0xff;
	case 2:
		return envini << 4 | envdir << 3 | envper;
	case 3:
		return clkpow << 4 | regwid << 3 | clkdiv;
	case 4:
		return uselen << 6 | 0xbf;
	default:
		std::cout << "Read from out of range noise channel register.\n";
		return 0xff;
	}
}

void SPU::NoiseChannel::WriteRegister(uint8_t reg, uint8_t value) {
	switch (reg) {
	case 0: break;
	case 1:
		sndlen = value & 0x1f;
		lengthtimer = 64 - sndlen;
		break;
	case 2:
		envini = (value >> 4) & 0xf;
		envdir = (value >> 3) & 0x1;
		envper = value & 0x7;
		if (envini == 0 && envdir == 0) {
			enable = false;
		}
		break;
	case 3:
		clkpow = (value >> 4) & 0xf;
		regwid = (value >> 3) & 0x1;
		clkdiv = value & 0x7;
		period = divtable[clkdiv] << clkpow;
		lfsrfeed = regwid ? 0x4040 : 0x4000;
		break;
	case 4:
		uselen = (value >> 6) & 0x1;
		if (value & 0x80) {
			Trigger();
		}
		break;
	default:
		std::cout << "Write to out of range noise channel register.\n";
	}
}

void SPU::NoiseChannel::Step() {
	periodtimer -= spuSampleClock;
	while (periodtimer <= 0 && period) {
		periodtimer += period;
		uint16_t tap = shiftregister;
		shiftregister >>= 1;
		tap ^= shiftregister;
		if (tap & 0x1) {
			shiftregister |= lfsrfeed;
		}
		else {
			shiftregister &= ~lfsrfeed;
		}
	}

	frametimer -= spuSampleClock;
	while (frametimer <= 0) {
		frametimer += 0x2000;
		TickFrame();
	}
}

void SPU::NoiseChannel::TickFrame() {
	frame = (frame + 1) % 8;
	if (uselen && (frame & 1) == 0 && lengthtimer > 0) {
		lengthtimer--;
		if (lengthtimer == 0) {
			enable = false;
		}
	}

	if (frame == 7 && envelopetimer != 0) {
		envelopetimer--;
		if (envelopetimer == 0) {
			int8_t newvolume = (int8_t)volume + (envdir ? 1 : -1);
			if (newvolume >= 0 && newvolume < 16) {
				envelopetimer = envper;
				volume = newvolume;
			}
		}
	}
}

int16_t SPU::NoiseChannel::Sample() {
	if (!enable) return 0;
	return (shiftregister & 0x1) ? volume : 0;
}

void SPU::NoiseChannel::Trigger() {
	enable = !(envper == 0 && envini == 0);
	lengthtimer = lengthtimer ? lengthtimer : 64;
	periodtimer = period;
	envelopetimer = envper;
	volume = envini;
	shiftregister = 0x7fff;
}