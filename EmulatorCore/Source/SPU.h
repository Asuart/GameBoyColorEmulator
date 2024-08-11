#pragma once
#include <cstdint>
#include "Bus.h"

class Bus;

static const uint32_t cpuFrequency = 70224 * 60;
static const uint32_t spuSampleRate = 32768;
static const uint32_t spuSampleClock = cpuFrequency / spuSampleRate;

class SPU {
public:
	SPU(Bus& bus);

	void Reset();
	void ClearSamples();
	uint32_t GetSamplesCount();
	int16_t* GetSamples();

	void Step(uint32_t cpuClocks);

	void WriteState(SaveState& state);
	void LoadState(uint8_t* state);

	Bus& bus;

	double clockAccumulator;
	uint32_t tickCounter;
	uint32_t sampleCounter;

	std::array<int16_t, spuSampleClock * 16> samples;

	union {
		struct {
			uint8_t ch1On : 1;
			uint8_t ch2On : 1;
			uint8_t ch3On : 1;
			uint8_t ch4On : 1;
			uint8_t amcUnused : 3;
			uint8_t audioOn : 1;
		};
		uint8_t audioMasterControl;
	};

	union {
		struct {
			uint8_t ch1Right : 1;
			uint8_t ch2Right : 1;
			uint8_t ch3Right : 1;
			uint8_t ch4Right : 1;
			uint8_t ch1Left : 1;
			uint8_t ch2Left : 1;
			uint8_t ch3Left : 1;
			uint8_t ch4Left : 1;
		};
		uint8_t soundPanning;
	};

	union {
		struct {
			uint8_t rightVolume : 3;
			uint8_t vInRight : 1;
			uint8_t leftVolume : 3;
			uint8_t vInLeft : 1;
		};
		uint8_t masterVolumeAndVIN;
	};

	struct ToneChannel {
		uint8_t wavsel = 0;
		uint8_t sndlen = 0;
		uint8_t envini = 0;
		uint8_t envdir = 0;
		uint8_t envper = 0;
		int16_t sndper = 0;
		uint8_t uselen = 0;

		bool enable = false;
		int16_t lengthtimer = 64;
		int16_t periodtimer = 0;
		int16_t envelopetimer = 0;
		int16_t period = 4;
		uint8_t waveframe = 0;
		int16_t frametimer = 0x2000;
		uint8_t frame = 0;
		uint8_t volume = 0;

		virtual uint8_t ReadRegister(uint8_t reg);
		virtual void WriteRegister(uint8_t reg, uint8_t value);
		virtual void Step();
		virtual void TickFrame();
		virtual int16_t Sample();
		virtual void Trigger();

	} toneChannel;

	struct SweepChannel : public ToneChannel {
		int16_t swpper = 0;
		uint8_t swpdir = 0;
		uint8_t swpmag = 0;

		int16_t sweeptimer = 0;
		bool sweepenable = false;
		int16_t shadow = 0;

		uint8_t ReadRegister(uint8_t reg) override;
		void WriteRegister(uint8_t reg, uint8_t value) override;
		void TickFrame() override;
		void Trigger() override;
		bool Sweep(bool save);

	} sweepChannel;

	struct WaveChannel {
		std::array<uint8_t, 0x10> wavetable = {0xff};
		
		uint8_t dacpow = 0;
		uint8_t sndlen = 0;
		int16_t volreg = 0;
		int16_t sndper = 0;
		uint8_t uselen = 0;

		bool enable = false;
		int16_t lengthtimer = 256;
		int16_t periodtimer = 0;
		int16_t period = 4;
		uint8_t waveframe = 0;
		int16_t frametimer = 0x2000;
		uint8_t frame = 0;
		uint8_t volumeshift = 0;

		uint8_t ReadRegister(uint8_t reg);
		void WriteRegister(uint8_t reg, uint8_t value);
		uint8_t ReadWaveByte(uint8_t offset);
		void WriteWaveByte(uint8_t offset, uint8_t value);
		void Step();
		void TickFrame();
		int16_t Sample();
		void Trigger();

	} waveChannel;

	struct NoiseChannel {
		const std::array<uint8_t, 8> divtable = { 8, 16, 32, 48, 64, 80, 96, 112 };

		uint8_t sndlen = 0;
		uint8_t envini = 0;
		uint8_t envdir = 0;
		uint8_t envper = 0;
		uint8_t clkpow = 0;
		uint8_t regwid = 0;
		uint8_t clkdiv = 0;
		uint8_t uselen = 0;

		bool enable = false;
		uint8_t lengthtimer = 64;
		int32_t periodtimer = 0;
		uint8_t envelopetimer = 0;
		int32_t period = 8;
		uint16_t shiftregister = 1;
		uint16_t lfsrfeed = 0x4000;
		int16_t frametimer = 0x2000;
		uint8_t frame = 0;
		uint8_t volume = 0;

		uint8_t ReadRegister(uint8_t reg);
		void WriteRegister(uint8_t reg, uint8_t value);
		void Step();
		void TickFrame();
		int16_t Sample();
		void Trigger();

	} noiseChannel;

public:
	uint8_t ReadNR50();
	uint8_t ReadNR51();
	uint8_t ReadNR52();
	uint8_t ReadPCM12();
	uint8_t ReadPCM34();

	void WriteNR50(uint8_t value);
	void WriteNR51(uint8_t value);
	void WriteNR52(uint8_t value);
};

