#include "PPU.h"

const Color palette[] = {
	Color(0.88f, 0.97f, 0.82f),
	Color(0.53f, 0.75f, 0.44f),
	Color(0.20f, 0.41f, 0.34f),
	Color(0.03f, 0.09f, 0.13f),
};

PPU::PPU(Bus& bus) : bus(bus) {}

void PPU::Reset() {
	dot = 0;
	LY = 0;
	LYC = 0;
	SCX = 0;
	SCXBuffer = 0;
	SCY = 0;
	WX = 0;
	WY = 0;
	LCDC = 0x91;
	STAT = 0x80;
	BGP = 0xe4;
	OBP0 = 0xff;
	OBP1 = 0xff;
	BGPI = 0xff;
	BGPD = 0xff;
	OBPI = 0xff;
	OBPD = 0xff;
	mode = 2;
	windowScanline = 0;
	frameReady = false;
	statInterruptionFlag = false;
}

void PPU::StepPixelMode(uint32_t cycles) {
	if (!LCDEnable) return;

	for (uint32_t i = 0; i < cycles; i++) {
		switch (mode) {
		case PPUMode::HBlank:
			if (!statInterruptionFlag && mode0IntSelect) {
				bus.TriggerInterruption(Interruption::LCD);
			}
			if (dot == (dotsPerScanline - 1)) {
				if (LY == (screenHeight - 1)) {
					mode = PPUMode::VBlank;
					frameReady = true;
					bus.TriggerInterruption(Interruption::VBlank);
				}
				else {
					mode = PPUMode::OAMScan;
				}
			}
			break;
		case PPUMode::VBlank:
			if (!statInterruptionFlag && mode1IntSelect) {
				bus.TriggerInterruption(Interruption::LCD);
			}
			if (dot == (dotsPerScanline - 1) && LY == (scanlineCount - 1)) {
				mode = PPUMode::OAMScan;
			}
			break;
		case PPUMode::OAMScan:
			if (!statInterruptionFlag && mode2IntSelect) {
				bus.TriggerInterruption(Interruption::LCD);
			}
			if (dot == 79) {
				mode = PPUMode::Rendering;
				SCX = SCXBuffer;
				UpdateMode3Penalty();
			}
			break;
		case PPUMode::Rendering:
			if ((dot - 80) < screenWidth) {
				if (dot % 8 == 0) SCX = (SCXBuffer & 0xf8) | (SCX & 0x07);
				DrawPixel();
			}
			else if (dot == (91 + screenWidth)) {
				mode = PPUMode::HBlank;
			}
			break;
		}

		dot++;
		if (dot == dotsPerScanline) {
			dot = 0;
			if (windowEnable && (WY <= LY) && (WX < 166)) windowScanline++;
			LY++;
			if (LY == scanlineCount) {
				LY = 0;
				windowScanline = 0;
				activeFrame = (activeFrame + 1) % 2;
				frameBuffers[activeFrame].Reset();
				statInterruptionFlag = false;
			}
		}

		LYCLY = (LY == LYC);
		if (!statInterruptionFlag && LYCLY && LYCIntSelect) {
			bus.TriggerInterruption(Interruption::LCD);
		}

		UpdateStatInterruptionFlag();
	}
}

void PPU::StepScanlineMode(uint32_t cycles) {
	//if (!LCDEnable) return;

	for (uint32_t i = 0; i < cycles; i++) {
		switch (mode) {
		case PPUMode::HBlank:
			if (!statInterruptionFlag && mode0IntSelect) {
				bus.TriggerInterruption(Interruption::LCD);
			}
			if (dot == (dotsPerScanline - 1)) {
				if (LY == (screenHeight - 1)) {
					mode = PPUMode::VBlank;
					frameReady = true;
					bus.TriggerInterruption(Interruption::VBlank);
				}
				else {
					mode = PPUMode::OAMScan;
				}
			}
			break;
		case PPUMode::VBlank:
			if (!statInterruptionFlag && mode1IntSelect) {
				bus.TriggerInterruption(Interruption::LCD);
			}
			if (dot == (dotsPerScanline - 1) && LY == (scanlineCount - 1)) {
				mode = PPUMode::OAMScan;
			}
			break;
		case PPUMode::OAMScan:
			if (!statInterruptionFlag && mode2IntSelect) {
				bus.TriggerInterruption(Interruption::LCD);
			}
			if (dot == 79) {
				mode = PPUMode::Rendering;
				SCX = SCXBuffer;
				PrerenderBGLine();
				PrerenderWindowLine();
				PrerenderSPRLine();
			}
			break;
		case PPUMode::Rendering:
			if ((dot - 80) < screenWidth) {
				if (dot % 8 == 0) SCX = (SCXBuffer & 0xf8) | (SCX & 0x07);
				MergePixel();
			}
			else if (dot == (80 + mode3Penalty + screenWidth)) {
				mode = PPUMode::HBlank;
			}
			break;
		}

		dot++;
		if (dot == dotsPerScanline) {
			dot = 0;
			if (windowEnable && (WY <= LY) && (WX < 166)) windowScanline++;
			LY++;
			if (LY == scanlineCount) {
				LY = 0;
				windowScanline = 0;
				activeFrame = (activeFrame + 1) % 2;
				frameBuffers[activeFrame].Reset();
				statInterruptionFlag = false;
			}
		}

		LYCLY = (LY == LYC);
		if (!statInterruptionFlag && LYCLY && LYCIntSelect) {
			bus.TriggerInterruption(Interruption::LCD);
		}

		UpdateStatInterruptionFlag();
	}
}

void PPU::DrawPixel() {
	uint8_t bgPixel = 0;
	uint8_t windowDrawn = false;
	if (windowEnable && (WY <= LY) && ((dot - 73) >= (WX))) {
		uint8_t x = dot - 80 - WX + 7;
		uint8_t y = windowScanline;
		uint8_t tileY = y / 8;
		uint8_t tileX = x / 8;
		uint16_t tileIndex = tileY * 32 + tileX;
		uint8_t tile = bus.Read8(0x9800 + tileIndex + 0x400 * windowTileMap);
		uint8_t low = 0, high = 0;
		if (BGAndWindowTileData) {
			low = bus.Read8(0x8000 + tile * 16 + (y % 8) * 2);
			high = bus.Read8(0x8000 + tile * 16 + (y % 8) * 2 + 1);
		}
		else {
			low = bus.Read8(0x9000 + (int8_t)tile * 16 + (y % 8) * 2);
			high = bus.Read8(0x9000 + (int8_t)tile * 16 + (y % 8) * 2 + 1);
		}
		bgPixel = ((low >> (7 - (x % 8))) & 1) | ((high >> (7 - (x % 8)) & 1) << 1);
		frameBuffers[activeFrame].SetPixel(dot - 80, LY, palette[(BGP >> (bgPixel * 2)) & 0b11]);
		windowDrawn = true;
	}
	if (!windowDrawn && BGAndWindowPriority) {
		uint8_t x = dot - 80 + SCX;
		uint8_t y = LY + SCY;
		uint8_t tileY = y / 8;
		uint8_t tileX = x / 8;
		uint16_t tileIndex = tileY * 32 + tileX;
		uint8_t tile = bus.Read8(0x9800 + tileIndex + 0x400 * BGTileMap);
		uint8_t low = 0, high = 0;
		if (BGAndWindowTileData) {
			low = bus.Read8(0x8000 + tile * 16 + (y % 8) * 2);
			high = bus.Read8(0x8000 + tile * 16 + (y % 8) * 2 + 1);
		}
		else {
			low = bus.Read8(0x9000 + (int8_t)tile * 16 + (y % 8) * 2);
			high = bus.Read8(0x9000 + (int8_t)tile * 16 + (y % 8) * 2 + 1);
		}
		bgPixel = ((low >> (7 - (x % 8))) & 1) | ((high >> (7 - (x % 8)) & 1) << 1);
		frameBuffers[activeFrame].SetPixel(dot - 80, LY, palette[(BGP >> (bgPixel * 2)) & 0b11]);
	}
	if (OBJEnable) {
		for (uint8_t spriteIndex = 0, objCount = 0, minX = 255; spriteIndex < 40 && objCount < 10; spriteIndex++) {
			OAMEntry* sprite = bus.GetOAMEntry(spriteIndex);
			if ((bgPixel && sprite->priority) || sprite->x >= minX) continue;
			uint16_t spriteX = sprite->x - 8;
			uint16_t spriteY = sprite->y - 16;
			if (LY < spriteY) continue;
			uint16_t row = LY - spriteY;
			uint16_t tileIndex = sprite->tile;
			if (OBJSize) {
				if (row > 15) continue;
				if (sprite->yFlip) row = 15 - row;
				tileIndex = (sprite->tile & 0xfe) + (row > 7);
			}
			else {
				if (row > 7) continue;
				if (sprite->yFlip) row = 7 - row;
			}
			objCount++;
			int16_t col = dot - 80 - spriteX;
			if (col < 0 || col > 7) continue;
			minX = sprite->x;
			if (sprite->xFlip) col = 7 - col;
			uint8_t low = bus.Read8(0x8000 + tileIndex * 16 + (row % 8) * 2);
			uint8_t high = bus.Read8(0x8000 + tileIndex * 16 + (row % 8) * 2 + 1);
			uint8_t color = (((low >> (7 - col)) & 1) | ((high >> (7 - col)) & 1) << 1);
			if (color == 0) continue;
			uint8_t pal = sprite->DMGPalette ? OBP1 : OBP0;
			frameBuffers[activeFrame].SetPixel(dot - 80, LY, palette[(pal >> (color * 2)) & 0b11]);
		}
	}
}

void PPU::DrawPatternTable(int32_t index) {
	for (int32_t y = 0; y < screenHeight; y++) {
		uint32_t tileY = y / 8;
		for (int32_t x = 0; x < screenWidth; x++) {
			uint32_t tileX = x / 8;
			uint32_t tileIndex = tileY * 32 + tileX;
			uint32_t tile = bus.Read8(0x9800 + tileIndex + 0x400 * index);
			uint8_t low = bus.Read8(0x8000 + tile * 16 + (y % 8) * 2);
			uint8_t high = bus.Read8(0x8000 + tile * 16 + (y % 8) * 2 + 1);
			uint8_t value = ((low >> (7 - (x % 8))) & 1) | ((high >> (7 - (x % 8)) & 1) << 1);
			float fValue = (float)value / 3.0f;
			frameBuffers[activeFrame].SetPixel(x, y, { fValue, fValue, fValue });
		}
	}
}

void PPU::UpdateMode3Penalty() {
	mode3Penalty = 12;
	mode3Penalty += SCX % 8;
	if (LY >= WY) mode3Penalty += 6;
	std::vector<OAMEntry*> scanlineSprites;
	for (int32_t i = 0; i < 40 && scanlineSprites.size() < 10; i++) {
		OAMEntry* sprite = bus.GetOAMEntry(i);
		uint16_t spriteY = sprite->y - 16;
		if (LY < spriteY) continue;
		uint16_t row = LY - spriteY;
		if (OBJSize && row > 15) continue;
		else if (row > 7) continue;
		scanlineSprites.push_back(sprite);
	}
	for (int32_t i = 0; i < scanlineSprites.size(); i++) {
		if (scanlineSprites[i]->x == 0) {
			mode3Penalty += 11;
			continue;
		}
		mode3Penalty += 4 + (7 - ((scanlineSprites[i]->x - (SCX % 8)) % 8));
	}
}


uint8_t PPU::ReadLCDC() {
	return LCDC;
}

uint8_t PPU::ReadSTAT() {
	return STAT;
}

uint8_t PPU::ReadSCY() {
	return SCY;
}

uint8_t PPU::ReadSCX() {
	return SCX;
}

uint8_t PPU::ReadLY() {
	return LY;
}

uint8_t PPU::ReadLYC() {
	return LYC;
}

uint8_t PPU::ReadBGP() {
	if (mode == PPUMode::Rendering) return 0xff;
	return BGP;
}

uint8_t PPU::ReadOBP0() {
	if (mode == PPUMode::Rendering) return 0xff;
	return OBP0;
}

uint8_t PPU::ReadOBP1() {
	if (mode == PPUMode::Rendering) return 0xff;
	return OBP1;
}

uint8_t PPU::ReadWY() {
	return WY;
}

uint8_t PPU::ReadWX() {
	return WX;
}

uint8_t PPU::ReadBGPI() {
	if (mode == PPUMode::Rendering) return 0xff;
	return BGPI;
}

uint8_t PPU::ReadBGPD() {
	if (mode == PPUMode::Rendering) return 0xff;
	return BGPD;
}

uint8_t PPU::ReadOBPI() {
	if (mode == PPUMode::Rendering) return 0xff;
	return OBPI;
}

uint8_t PPU::ReadOBPD() {
	if (mode == PPUMode::Rendering) return 0xff;
	return OBPD;
}

uint8_t PPU::ReadOPRI() {
	return OPRI;
}

void PPU::WriteLCDC(uint8_t value) {
	LCDC = value;
	if (!LCDEnable) {
		LY = 0;
		dot = 0;
		mode = PPUMode::OAMScan;
	}
}

void PPU::WriteSTAT(uint8_t value) {
	STAT = (value & 0xf8) | (STAT & 0x7) | 0x80;
}

void PPU::WriteSCY(uint8_t value) {
	SCY = value;
}

void PPU::WriteSCX(uint8_t value) {
	SCXBuffer = value;
}

void PPU::WriteLY(uint8_t value) {
	std::cout << "Warning: write to read only register LY\n";
}

void PPU::WriteLYC(uint8_t value) {
	LYC = value;
}

void PPU::WriteBGP(uint8_t value) {
	if (mode == PPUMode::Rendering) return;
	BGP = value;
}

void PPU::WriteOBP0(uint8_t value) {
	if (mode == PPUMode::Rendering) return;
	OBP0 = value;
}

void PPU::WriteOBP1(uint8_t value) {
	if (mode == PPUMode::Rendering) return;
	OBP1 = value;
}

void PPU::WriteWY(uint8_t value) {
	WY = value;
}

void PPU::WriteWX(uint8_t value) {
	WX = value;
}

void PPU::WriteBGPI(uint8_t value) {
	if (mode == PPUMode::Rendering) return;
	BGPI = value;
}

void PPU::WriteBGPD(uint8_t value) {
	if (mode == PPUMode::Rendering) return;
	BGPD = value;
}

void PPU::WriteOBPI(uint8_t value) {
	if (mode == PPUMode::Rendering) return;
	OBPI = value;
}

void PPU::WriteOBPD(uint8_t value) {
	if (mode == PPUMode::Rendering) return;
	OBPD = value;
}

void PPU::WriteOPRI(uint8_t value) {
	if (mode == PPUMode::Rendering) return;
	OPRI = value;
}

void PPU::WriteState(SaveState& state) {
	state.Write16(dot);
	state.Write8(mode3Penalty);
	state.Write8(windowScanline);
	state.Write8(statInterruptionFlag);
	state.Write8(LCDC);
	state.Write8(STAT);
	state.Write8(SCY);
	state.Write8(SCX);
	state.Write8(SCXBuffer);
	state.Write8(LY);
	state.Write8(LYC);
	state.Write8(BGP);
	state.Write8(OBP0);
	state.Write8(OBP1);
	state.Write8(WY);
	state.Write8(WX);
	state.Write8(BGPI);
	state.Write8(BGPD);
	state.Write8(OBPI);
	state.Write8(OBPD);
	state.Write8(OPRI);
}

void PPU::LoadState(SaveState& state) {
	dot = state.Read16();
	mode3Penalty = state.Read8();
	windowScanline = state.Read8();
	statInterruptionFlag = state.Read8();
	LCDC = state.Read8();
	STAT = state.Read8();
	SCY = state.Read8();
	SCX = state.Read8();
	SCXBuffer = state.Read8();
	LY = state.Read8();
	LYC = state.Read8();
	BGP = state.Read8();
	OBP0 = state.Read8();
	OBP1 = state.Read8();
	WY = state.Read8();
	WX = state.Read8();
	BGPI = state.Read8();
	BGPD = state.Read8();
	OBPI = state.Read8();
	OBPD = state.Read8();
	OPRI = state.Read8();
}

void PPU::UpdateStatInterruptionFlag() {
	statInterruptionFlag = LYCLY;
	if (statInterruptionFlag) return;
	switch (mode) {
	case 0: statInterruptionFlag = mode0IntSelect; break;
	case 1: statInterruptionFlag = mode1IntSelect; break;
	case 2: statInterruptionFlag = mode2IntSelect; break;
	}
}

void PPU::PrerenderBGLine() {
	memset(bgLinePixels.data(), 0, bgLinePixels.size() * sizeof(bgLinePixels[0]));

	for (uint32_t i = 0, x = 0; i < 32; i++, x += 8) {
		if (BGAndWindowPriority) {
			uint8_t y = LY + SCY;
			uint16_t tileIndex = ((uint16_t)(y & 0xf8) << 2) + (x >> 3);
			uint8_t tile = bus.Read8(0x9800 + tileIndex + 0x400 * BGTileMap);
			uint8_t low = 0, high = 0;
			if (BGAndWindowTileData) {
				low = bus.Read8(0x8000 + tile * 16 + (y % 8) * 2);
				high = bus.Read8(0x8000 + tile * 16 + (y % 8) * 2 + 1);
			}
			else {
				low = bus.Read8(0x9000 + (int8_t)tile * 16 + (y % 8) * 2);
				high = bus.Read8(0x9000 + (int8_t)tile * 16 + (y % 8) * 2 + 1);
			}
			for (uint8_t pixel = 0; pixel < 8; pixel++) {
				bgLinePixels[x + pixel] = ((low >> (7 - pixel)) & 1) | ((high >> (7 - pixel) & 1) << 1);
			}
		}
	}
}

void PPU::PrerenderWindowLine() {
	if (!windowEnable || WY > LY || WX >= 166) return;
	memset(windowLinePixels.data(), 0, windowLinePixels.size() * sizeof(windowLinePixels[0]));

	for (uint32_t i = 0, x = 0; i < 20; i++, x += 8) {
		uint8_t y = windowScanline;
		uint16_t tileIndex = ((uint16_t)(y & 0xf8) << 2) + (x >> 3);
		uint8_t tile = bus.Read8(0x9800 + tileIndex + 0x400 * windowTileMap);
		uint8_t low = 0, high = 0;
		if (BGAndWindowTileData) {
			low = bus.Read8(0x8000 + tile * 16 + (y % 8) * 2);
			high = bus.Read8(0x8000 + tile * 16 + (y % 8) * 2 + 1);
		}
		else {
			low = bus.Read8(0x9000 + (int8_t)tile * 16 + (y % 8) * 2);
			high = bus.Read8(0x9000 + (int8_t)tile * 16 + (y % 8) * 2 + 1);
		}
		for (uint8_t pixel = 0; pixel < 8; pixel++) {
			windowLinePixels[x + pixel] = ((low >> (7 - pixel)) & 1) | ((high >> (7 - pixel) & 1) << 1);
		}
	}
}

void PPU::PrerenderSPRLine() {
	memset(sprLinePixels.data(), 0, sprLinePixels.size() * sizeof(sprLinePixels[0]));

	for (uint8_t spriteIndex = 0, objCount = 0; spriteIndex < 40 && objCount < 10; spriteIndex++) {
		OAMEntry* sprite = bus.GetOAMEntry(spriteIndex);
		int16_t spriteX = sprite->x - 8;
		int16_t spriteY = sprite->y - 16;
		if (LY < spriteY) continue;
		uint16_t row = LY - spriteY;
		uint16_t tileIndex = sprite->tile;
		if (OBJSize) {
			if (row > 15) continue;
			if (sprite->yFlip) row = 15 - row;
			tileIndex = (sprite->tile & 0xfe) + (row > 7);
		}
		else {
			if (row > 7) continue;
			if (sprite->yFlip) row = 7 - row;
		}
		objCount++;

		uint8_t low = bus.Read8(0x8000 + tileIndex * 16 + (row % 8) * 2);
		uint8_t high = bus.Read8(0x8000 + tileIndex * 16 + (row % 8) * 2 + 1);
		uint8_t pal = sprite->DMGPalette ? OBP1 : OBP0;

		if (sprite->xFlip) {
			for (uint8_t pixelX = 7, x = 0; x < 8; pixelX--, x++) {
				if ((spriteX + x) >= sprLinePixels.size() || (sprLinePixels[spriteX + x] & 0b11) != 0) continue;
				uint8_t color = (((low >> (7 - pixelX)) & 1) | ((high >> (7 - pixelX)) & 1) << 1);
				if (color == 0) continue;
				sprLinePixels[spriteX + x] = (sprite->priority << 2) | color | (pal << 8);// palette[(pal >> (color * 2)) & 0b11];
			}
		}
		else {
			for (uint8_t pixelX = 0; pixelX < 8; pixelX++) {
				if ((spriteX + pixelX) >= sprLinePixels.size() || (sprLinePixels[spriteX + pixelX] & 0b11) != 0) continue;
				uint8_t color = (((low >> (7 - pixelX)) & 1) | ((high >> (7 - pixelX)) & 1) << 1);
				if (color == 0) continue;
				sprLinePixels[spriteX + pixelX] = (sprite->priority << 2) | color | (pal << 8); // palette[(pal >> (color * 2)) & 0b11];
			}
		}
	}
}

void PPU::MergePixel() {

	if (windowEnable && WY <= LY && WX < 160 && (dot - 73) >= WX) {
		if (OBJEnable && (sprLinePixels[dot - 80] & 0b11) != 0 && (!((sprLinePixels[dot - 80] >> 2) & 1) || !windowLinePixels[(dot - 73) - WX])) {
			frameBuffers[activeFrame].SetPixel(dot - 80, LY, palette[((sprLinePixels[dot - 80] >> 8) >> ((sprLinePixels[dot - 80] & 0b11) * 2)) & 0b11]);
		}
		else {
			frameBuffers[activeFrame].SetPixel(dot - 80, LY, palette[(BGP >> (windowLinePixels[(dot - 73) - WX] * 2)) & 0b11]);
		}
	}
	else {
		if (OBJEnable && (sprLinePixels[dot - 80] & 0b11) != 0 && (!((sprLinePixels[dot - 80] >> 2) & 1) || !bgLinePixels[(dot - 80 + SCX) % bgLinePixels.size()])) {
			frameBuffers[activeFrame].SetPixel(dot - 80, LY, palette[((sprLinePixels[dot - 80] >> 8) >> ((sprLinePixels[dot - 80] & 0b11) * 2)) & 0b11]);
		}
		else {
			frameBuffers[activeFrame].SetPixel(dot - 80, LY, palette[(BGP >> (bgLinePixels[(dot - 80 + SCX) % bgLinePixels.size()] * 2)) & 0b11]);
		}
	}
}