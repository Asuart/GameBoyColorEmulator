#pragma once
#include <cstdint>
#include "Bus.h"
#include "Texture.h"

class Bus;

enum PPUMode {
	HBlank = 0,
	VBlank = 1,
	OAMScan = 2,
	Rendering = 3,
};

class PPU {
public:
	static const uint32_t scanlineCount = 154;
	static const uint32_t dotsPerScanline = 456;
	static const uint32_t screenWidth = 160;
	static const uint32_t screenHeight = 144;

	uint8_t activeFrame = 0;
	Texture<Color> frameBuffers[2] = {
		Texture<Color>(screenWidth, screenHeight),
		Texture<Color>(screenWidth, screenHeight)
	};

	PPU(Bus& bus);

	void Reset();
	void StepPixelMode(uint32_t cycles);
	void StepScanlineMode(uint32_t cycles);

	void DrawPatternTable(int32_t index);

	uint8_t ReadLCDC();
	uint8_t ReadSTAT();
	uint8_t ReadSCY();
	uint8_t ReadSCX();
	uint8_t ReadLY();
	uint8_t ReadLYC();
	uint8_t ReadBGP();
	uint8_t ReadOBP0();
	uint8_t ReadOBP1();
	uint8_t ReadWY();
	uint8_t ReadWX();
	uint8_t ReadBGPI();
	uint8_t ReadBGPD();
	uint8_t ReadOBPI();
	uint8_t ReadOBPD();
	uint8_t ReadOPRI();
	void WriteLCDC(uint8_t value);
	void WriteSTAT(uint8_t value);
	void WriteSCY(uint8_t value);
	void WriteSCX(uint8_t value);
	void WriteLY(uint8_t value);
	void WriteLYC(uint8_t value);
	void WriteBGP(uint8_t value);
	void WriteOBP0(uint8_t value);
	void WriteOBP1(uint8_t value);
	void WriteWY(uint8_t value);
	void WriteWX(uint8_t value);
	void WriteBGPI(uint8_t value);
	void WriteBGPD(uint8_t value);
	void WriteOBPI(uint8_t value);
	void WriteOBPD(uint8_t value);
	void WriteOPRI(uint8_t value);

	Bus& bus;

	uint32_t dot = 0;
	uint8_t mode3Penalty = 12;
	uint8_t windowScanline = 0;
	bool frameReady = false;
	bool statInterruptionFlag = false;

	union {
		uint8_t LCDC;
		struct {
			uint8_t BGAndWindowPriority : 1;
			uint8_t OBJEnable : 1;
			uint8_t OBJSize : 1;
			uint8_t BGTileMap : 1;
			uint8_t BGAndWindowTileData : 1;
			uint8_t windowEnable : 1;
			uint8_t windowTileMap : 1;
			uint8_t LCDEnable : 1;
		};
	};
	union {
		uint8_t STAT;
		struct {
			uint8_t mode : 2;
			uint8_t LYCLY : 1;
			uint8_t mode0IntSelect : 1;
			uint8_t mode1IntSelect : 1;
			uint8_t mode2IntSelect : 1;
			uint8_t LYCIntSelect : 1;
		};
	};
	uint8_t SCY;
	uint8_t SCX;
	uint8_t SCXBuffer;
	uint8_t LY;
	uint8_t LYC;
	uint8_t BGP;
	uint8_t OBP0;
	uint8_t OBP1;
	uint8_t WY;
	uint8_t WX;
	uint8_t BGPI;
	uint8_t BGPD;
	uint8_t OBPI;
	uint8_t OBPD;
	uint8_t OPRI;

	std::array<uint8_t, 256> bgLinePixels;
	std::array<uint8_t, 166> windowLinePixels;
	std::array<uint16_t, screenWidth> sprLinePixels;

	void DrawPixel();
	void UpdateMode3Penalty();
	void PrerenderBGLine();
	void PrerenderWindowLine();
	void PrerenderSPRLine();
	void MergePixel();
	void UpdateStatInterruptionFlag();

	friend class Bus;
	friend class GBCEmulator;
};