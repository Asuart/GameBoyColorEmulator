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

	void Step(uint32_t cpuCLocks);
	void Tick();
	void TickSoundLength();
	void TickFrequencySweep();
	void TickEnvelopeSweep();

	Bus& bus;

	double clockAccumulator;
	uint32_t tickCounter;
	uint32_t sampleCounter;

	std::array<int16_t, spuSampleClock * 4> samples;

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

	struct PulseChannel {
		union {
			uint8_t sweep;
			struct {
				uint8_t individualStep : 3;
				uint8_t direction : 1;
				uint8_t pace : 3;
				uint8_t sweepUnused : 1;
			};
		};
		uint8_t activePace;

		union {
			uint8_t lengthAndDuty;
			struct {
				uint8_t initialLengthTimer : 6;
				uint8_t waveDuty : 2;
			};
		};

		union {
			uint8_t volumeAndEnvelope;
			struct {
				uint8_t sweepPace : 3;
				uint8_t envelopeDirection : 1;
				uint8_t initialVolume : 4;
			};
		};

		uint8_t periodLow;

		union {
			uint8_t periodHighAndControl;
			struct {
				uint8_t period : 3;
				uint8_t controlUnused : 3;
				uint8_t lengthEnabled : 1;
				uint8_t trigger : 1;
			};
		};

		bool active;
		uint8_t periodLowBuffer, periodHighBuffer;
		int8_t lengthTimer;
		uint8_t waveCycle;
		int32_t periodTimer;
		int32_t frameTimer;
		int32_t envelopeTimer;
		int32_t frame;
		int32_t volume;
		int32_t soundPeriod;
		int32_t framePeriod;


		PulseChannel();
		void Step();
		void TickFrame();
		int16_t Sample();
		void Trigger();

	} pulse1, pulse2;

	struct WaveChannel {
		union {
			uint8_t dacEnable;
			struct {
				uint8_t dacEnableUnused : 7;
				uint8_t dacOn : 1;
			};
		};

		uint8_t lengthTimer;

		union {
			uint8_t outputLevel;
			struct {
				uint8_t outputLevelUnused0 : 5;
				uint8_t outputLevelBits : 2;
				uint8_t outputLevelUnused1 : 1;
			};
		};

		uint8_t periodLow;
		uint8_t periodLowBuffer, periodHighBuffer;

		union {
			uint8_t periodHighAndControl;
			struct {
				uint8_t period : 3;
				uint8_t periodHighAndControlUnused : 3;
				uint8_t lengthEnable : 1;
				uint8_t trigger : 1;
			};
		};

		std::array<uint8_t, 0x10> pattern;

		void Trigger();

	} wave;

	struct NoiseChannel {
		union {
			uint8_t lengthTimer;
			struct {
				uint8_t initialLengthTimer : 6;
				uint8_t lengthTimerUnused : 2;
			};
		};

		union {
			uint8_t volumeAndEnvelope;
			struct {
				uint8_t sweepPace : 3;
				uint8_t envelopeDirection : 1;
				uint8_t initialVolume : 4;
			};
		};

		union {
			uint8_t frequencyAndRandomness;
			struct {
				uint8_t clockDivider : 3;
				uint8_t lfsrWifth : 1;
				uint8_t clockShift : 4;
			};
		};

		union {
			uint8_t control;
			struct {
				uint8_t controlUnused : 6;
				uint8_t lengthEnable : 1;
				uint8_t trigger : 1;
			};
		};

		void Trigger();

	} noise;

public:
	uint8_t ReadNR10();
	uint8_t ReadNR11();
	uint8_t ReadNR12();
	uint8_t ReadNR13();
	uint8_t ReadNR14();
	uint8_t ReadNR21();
	uint8_t ReadNR22();
	uint8_t ReadNR23();
	uint8_t ReadNR24();
	uint8_t ReadNR30();
	uint8_t ReadNR31();
	uint8_t ReadNR32();
	uint8_t ReadNR33();
	uint8_t ReadNR34();
	uint8_t ReadNR41();
	uint8_t ReadNR42();
	uint8_t ReadNR43();
	uint8_t ReadNR44();
	uint8_t ReadNR50();
	uint8_t ReadNR51();
	uint8_t ReadNR52();
	uint8_t ReadWavePattern(uint16_t address);
	uint8_t ReadPCM12();
	uint8_t ReadPCM34();

	void WriteNR10(uint8_t value);
	void WriteNR11(uint8_t value);
	void WriteNR12(uint8_t value);
	void WriteNR13(uint8_t value);
	void WriteNR14(uint8_t value);
	void WriteNR21(uint8_t value);
	void WriteNR22(uint8_t value);
	void WriteNR23(uint8_t value);
	void WriteNR24(uint8_t value);
	void WriteNR30(uint8_t value);
	void WriteNR31(uint8_t value);
	void WriteNR32(uint8_t value);
	void WriteNR33(uint8_t value);
	void WriteNR34(uint8_t value);
	void WriteNR41(uint8_t value);
	void WriteNR42(uint8_t value);
	void WriteNR43(uint8_t value);
	void WriteNR44(uint8_t value);
	void WriteNR50(uint8_t value);
	void WriteNR51(uint8_t value);
	void WriteNR52(uint8_t value);
	void WriteWavePattern(uint16_t address, uint8_t value);
};

