#include "CPU.h"

CPU::CPU(Bus& bus) : bus(bus) {}

void CPU::Reset() {
	AF = 0x01b0;
	BC = 0x0013;
	DE = 0x00d8;
	HL = 0x014d;
	PC = 0x100;
	SP = 0xfffe;
	IME = 0;
	isHalting = false;
	haltBug = false;
}

uint32_t CPU::Step() {
	if (HandleInterruptions()) return 20;
	if (isHalting) return 4;
	opcode = Read8(PC);
	if(logAssembler) std::cout << std::hex << "0x" << PC << ": " << baseOpcodeTable[opcode].assembler << " BC=" << BC << " DE=" << DE << " HL=" << HL << " AF=" << AF << " SP=" << SP << "\n";

	if (!haltBug) PC++;
	haltBug = false;
	switch (baseOpcodeTable[opcode].length) {
	case 2:
		attr8 = Read8(PC);
		PC++;
		break;
	case 3:
		attr16 = Read16(PC);
		PC += 2;
		break;
	}
	dummyRead = bus.Read8(PC);
	stepClock += opcodeTimingTable[opcode] * 4;
	//ExecuteThroughTable();
	ExecuteInline();

	uint32_t cpuCycles = stepClock;
	clock += stepClock;
	stepClock = 0;
	return cpuCycles;
}

bool CPU::HandleInterruptions() {
	uint8_t interruptions = bus.IF & bus.IE & 0x1f;
	if (!interruptions) return false;
	isHalting = false;
	if (!IME) return false;

	if (interruptions & (uint8_t)Interruption::VBlank) {
		bus.IF &= ~(uint8_t)Interruption::VBlank;
		HandleInterruption(0x40);
	}
	else if (interruptions & (uint8_t)Interruption::LCD) {
		bus.IF &= ~(uint8_t)Interruption::LCD;
		HandleInterruption(0x48);
	}
	else if (interruptions & (uint8_t)Interruption::Timer) {
		bus.IF &= ~(uint8_t)Interruption::Timer;
		HandleInterruption(0x50);
	}
	else if (interruptions & (uint8_t)Interruption::Serial) {
		bus.IF &= ~(uint8_t)Interruption::Serial;
		HandleInterruption(0x58);
	}
	else if (interruptions & (uint8_t)Interruption::Joypad) {
		bus.IF &= ~(uint8_t)Interruption::Joypad;
		HandleInterruption(0x60);
	}

	return true;
}

void CPU::HandleInterruption(uint16_t handlerAddress) {
	IME = 0;
	Push16(PC);
	PC = handlerAddress;
}

// Help Instructions
uint8_t CPU::GetR8(uint8_t index) {
	switch (index & 0b111) {
	case 0:
		return B;
	case 1:
		return C;
	case 2:
		return D;
	case 3:
		return E;
	case 4:
		return H;
	case 5:
		return L;
	case 6:
		std::cout << "Error: Read from register at index 6 should be using [HL] instructions.\n";
		return 0;
	case 7:
		return A;
	}
	return 0;
}

void CPU::SetR8(uint8_t index, uint8_t value) {
	switch (index & 0b111) {
	case 0:
		B = value;
		break;
	case 1:
		C = value;
		break;
	case 2:
		D = value;
		break;
	case 3:
		E = value;
		break;
	case 4:
		H = value;
		break;
	case 5:
		L = value;
		break;
	case 6:
		std::cout << "Error: Write to register at index 6 should be using [HL] instructions.\n";
		break;
	case 7:
		A = value;
		break;
	}
}

uint16_t CPU::GetR16(uint8_t index) {
	switch (index & 0b11) {
	case 0:
		return BC;
	case 1:
		return DE;
	case 2:
		return HL;
	case 3:
		if (opcode >> 7) {
			return AF;
		}
		else {
			return SP;
		}
	}
	return 0;
}

void CPU::SetR16(uint8_t index, uint16_t value) {
	switch (index & 0b11) {
	case 0:
		BC = value;
		break;
	case 1:
		DE = value;
		break;
	case 2:
		HL = value;
		break;
	case 3:
		if (opcode >> 7) {
			AF = value & 0xfff0;
		}
		else {
			SP = value;
		}
		break;
	}
}

uint8_t CPU::Read8(uint16_t address) {
	return bus.Read8(address);
}

void CPU::Write8(uint16_t address, uint8_t value) {
	bus.Write8(address, value);
}

uint16_t CPU::Read16(uint16_t address) {
	return bus.Read16(address);
}

void CPU::Write16(uint16_t address, uint16_t value) {
	bus.Write16(address, value);
}

uint16_t CPU::Pop16() {
	SP += 2;
	return bus.Read16(SP - 2);
}

void CPU::Push16(uint16_t value) {
	SP -= 2;
	bus.Write16(SP, value);
}

bool CPU::CheckCondition(uint8_t condition) {
	switch (condition & 0b11) {
	case 0:
		return !fZero;
	case 1:
		return fZero;
	case 2:
		return !fCarry;
	case 3:
		return fCarry;
	}
	return false;
}

void CPU::ExecuteThroughTable() {
	(this->*baseOpcodeTable[opcode].inst)();
}

void CPU::ExecuteInline() {
	uint32_t result, temp;
	
	switch (opcode) {
	case 0x00:
		break;
	case 0x01:
		BC = attr16;
		break;
	case 0x02:
		Write8(BC, A);
		break;
	case 0x03:
		BC++;
		break;
	case 0x04:
		fSub = 0;
		fHalfCarry = (B & 0xf) == 0xf;
		B++;
		fZero = (B == 0);
		break;
	case 0x05:
		fSub = 1;
		fHalfCarry = (B & 0xf) == 0;
		B--;
		fZero = (B == 0);
		break;
	case 0x06:
		B = attr8;
		break;
	case 0x07:
		A = (A << 1) | (A >> 7);
		fZero = 0;
		fSub = 0;
		fHalfCarry = 0;
		fCarry = A & 1;
		break;
	case 0x08:
		Write16(attr16, SP);
		break;
	case 0x09:
		result = HL + BC;
		fSub = 0;
		fHalfCarry = ((HL & 0x0fff) + (BC & 0x0fff)) > 0x0fff;
		fCarry = result > 0xffff;
		HL = result;
		break;
	case 0x0a:
		A = Read8(BC);
		break;
	case 0x0b:
		BC--;
		break;
	case 0x0c:
		fSub = 0;
		fHalfCarry = (C & 0xf) == 0xf;
		C++;
		fZero = (C == 0);
		break;
	case 0x0d:
		fSub = 1;
		fHalfCarry = (C & 0xf) == 0;
		C--;
		fZero = (C == 0);
		break;
	case 0x0e:
		C = attr8;
		break;
	case 0x0f:
		A = (A >> 1) | (A << 7);
		fCarry = A >> 7;
		fZero = 0;
		fSub = 0;
		fHalfCarry = 0;
		break;
	case 0x10:
		Write8(0xff04, 0);
		break;
	case 0x11:
		DE = attr16;
		break;
	case 0x12:
		Write8(DE, A);
		break;
	case 0x13:
		DE++;
		break;
	case 0x14:
		fSub = 0;
		fHalfCarry = (D & 0xf) == 0xf;
		D++;
		fZero = (D == 0);
		break;
	case 0x15:
		fSub = 1;
		fHalfCarry = (D & 0xf) == 0;
		D--;
		fZero = (D == 0);
		break;
	case 0x16:
		D = attr8;
		break;
	case 0x017:
		temp = A >> 7;
		A = (A << 1) | fCarry;
		fZero = 0;
		fSub = 0;
		fHalfCarry = 0;
		fCarry = temp;
		break;
	case 0x18:
		PC += (int8_t)attr8;
		break;
	case 0x19:
		result = HL + DE;
		fSub = 0;
		fHalfCarry = ((HL & 0x0fff) + (DE & 0x0fff)) > 0x0fff;
		fCarry = result > 0xffff;
		HL = result;
		break;
	case 0x1a:
		A = Read8(DE);
		break;
	case 0x1b:
		DE--;
		break;
	case 0x1c:
		fSub = 0;
		fHalfCarry = (E & 0xf) == 0xf;
		E++;
		fZero = (E == 0);
		break;
	case 0x1d:
		fSub = 1;
		fHalfCarry = (E & 0xf) == 0;
		E--;
		fZero = (E == 0);
		break;
	case 0x1e:
		E = attr8;
		break;
	case 0x1f:
		temp = A & 0b1;
		A = (A >> 1) | (fCarry << 7);
		fZero = 0;
		fSub = 0;
		fHalfCarry = 0;
		fCarry = temp;
		break;
	case 0x20:
		if (!fZero) {
			PC += (int8_t)attr8;
			stepClock += 4;
		}
		break;
	case 0x21:
		HL = attr16;
		break;
	case 0x22:
		Write8(HL, A);
		HL++;
		break;
	case 0x23:
		HL++;
		break;
	case 0x24:
		fSub = 0;
		fHalfCarry = (H & 0xf) == 0xf;
		H++;
		fZero = (H == 0);
		break;
	case 0x25:
		fSub = 1;
		fHalfCarry = (H & 0xf) == 0;
		H--;
		fZero = (H == 0);
		break;
	case 0x26:
		H = attr8;
		break;
	case 0x27:
		if (!fSub) {
			if (fCarry || A > 0x99) { A += 0x60; fCarry = 1; }
			if (fHalfCarry || (A & 0x0f) > 0x09) { A += 0x6; }
		}
		else {
			if (fCarry) { A -= 0x60; }
			if (fHalfCarry) { A -= 0x6; }
		}
		fZero = (A == 0);
		fHalfCarry = 0;
		break;
	case 0x28:
		if (fZero) {
			PC += (int8_t)attr8;
			stepClock += 4;
		}
		break;
	case 0x29:
		result = HL + HL;
		fSub = 0;
		fHalfCarry = ((HL & 0x0fff) + (HL & 0x0fff)) > 0x0fff;
		fCarry = result > 0xffff;
		HL = result;
		break;
	case 0x2a:
		A = Read8(HL);
		HL++;
		break;
	case 0x2b:
		HL--;
		break;
	case 0x2c:
		fSub = 0;
		fHalfCarry = (L & 0xf) == 0xf;
		L++;
		fZero = (L == 0);
		break;
	case 0x2d:
		fSub = 1;
		fHalfCarry = (L & 0xf) == 0;
		L--;
		fZero = (L == 0);
		break;
	case 0x2e:
		L = attr8;
		break;
	case 0x2f:
		A = ~A;
		fSub = 1;
		fHalfCarry = 1;
		break;
	case 0x30:
		if (!fCarry) {
			PC += (int8_t)attr8;
			stepClock += 4;
		}
		break;
	case 0x31:
		SP = attr16;
		break;
	case 0x32:
		Write8(HL, A);
		HL--;
		break;
	case 0x33:
		SP++;
		break;
	case 0x34:
		temp = Read8(HL);
		result = (temp + 1) & 0xff;
		Write8(HL, result);
		fZero = (result == 0);
		fSub = 0;
		fHalfCarry = (temp & 0xf) == 0xf;
		break;
	case 0x35:
		temp = Read8(HL);
		result = (temp - 1) & 0xff;
		Write8(HL, result);
		fZero = (result == 0);
		fSub = 1;
		fHalfCarry = (temp & 0xf) == 0;
		break;
	case 0x36:
		Write8(HL, attr8);
		break;
	case 0x37:
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 1;
		break;
	case 0x38:
		if (fCarry) {
			PC += (int8_t)attr8;
			stepClock += 4;
		}
		break;
	case 0x39:
		result = HL + SP;
		fSub = 0;
		fHalfCarry = ((HL & 0x0fff) + (SP & 0x0fff)) > 0x0fff;
		fCarry = result > 0xffff;
		HL = result;
		break;
	case 0x3a:
		A = Read8(HL);
		HL--;
		break;
	case 0x3b:
		SP--;
		break;
	case 0x3c:
		fSub = 0;
		fHalfCarry = (A & 0xf) == 0xf;
		A++;
		fZero = (A == 0);
		break;
	case 0x3d:
		fSub = 1;
		fHalfCarry = (A & 0xf) == 0;
		A--;
		fZero = (A == 0);
		break;
	case 0x3e:
		A = attr8;
		break;
	case 0x3f:
		fSub = 0;
		fHalfCarry = 0;
		fCarry = !fCarry;
		break;
	case 0x40:
		break;
	case 0x41:
		B = C;
		break;
	case 0x42:
		B = D;
		break;
	case 0x43:
		B = E;
		break;
	case 0x44:
		B = H;
		break;
	case 0x45:
		B = L;
		break;
	case 0x46:
		B = Read8(HL);
		break;
	case 0x47:
		B = A;
		break;
	case 0x48:
		C = B;
		break;
	case 0x49:
		break;
	case 0x4a:
		C = D;
		break;
	case 0x4b:
		C = E;
		break;
	case 0x4c:
		C = H;
		break;
	case 0x4d:
		C = L;
		break;
	case 0x4e:
		C = Read8(HL);
		break;
	case 0x4f:
		C = A;
		break;
	case 0x50:
		D = B;
		break;
	case 0x51:
		D = C;
		break;
	case 0x52:
		break;
	case 0x53:
		D = E;
		break;
	case 0x54:
		D = H;
		break;
	case 0x55:
		D = L;
		break;
	case 0x56:
		D = Read8(HL);
		break;
	case 0x57:
		D = A;
		break;
	case 0x58:
		E = B;
		break;
	case 0x59:
		E = C;
		break;
	case 0x5a:
		E = D;
		break;
	case 0x5b:
		break;
	case 0x5c:
		E = H;
		break;
	case 0x5d:
		E = L;
		break;
	case 0x5e:
		E = Read8(HL);
		break;
	case 0x5f:
		E = A;
		break;
	case 0x60:
		H = B;
		break;
	case 0x61:
		H = C;
		break;
	case 0x62:
		H = D;
		break;
	case 0x63:
		H = E;
		break;
	case 0x64:
		break;
	case 0x65:
		H = L;
		break;
	case 0x66:
		H = Read8(HL);
		break;
	case 0x67:
		H = A;
		break;
	case 0x68:
		L = B;
		break;
	case 0x69:
		L = C;
		break;
	case 0x6a:
		L = D;
		break;
	case 0x6b:
		L = E;
		break;
	case 0x6c:
		L = H;
		break;
	case 0x6d:
		break;
	case 0x6e:
		L = Read8(HL);
		break;
	case 0x6f:
		L = A;
		break;
	case 0x70:
		Write8(HL, B);
		break;
	case 0x71:
		Write8(HL, C);
		break;
	case 0x72:
		Write8(HL, D);
		break;
	case 0x73:
		Write8(HL, E);
		break;
	case 0x74:
		Write8(HL, H);
		break;
	case 0x75:
		Write8(HL, L);
		break;
	case 0x76:
		if (IME || (bus.IE & bus.IF & 0x1f) == 0) {
			isHalting = true;
		}
		else {
			haltBug = true;
		}
		stepClock += 4;
		break;
	case 0x77:
		Write8(HL, A);
		break;
	case 0x78:
		A = B;
		break;
	case 0x79:
		A = C;
		break;
	case 0x7a:
		A = D;
		break;
	case 0x7b:
		A = E;
		break;
	case 0x7c:
		A = H;
		break;
	case 0x7d:
		A = L;
		break;
	case 0x7e:
		A = Read8(HL);
		break;
	case 0x7f:
		break;
	case 0x80:
		result = A + B;
		fZero = ((result & 0xff) == 0);
		fSub = 0;
		fHalfCarry = ((A & 0x0f) + (B & 0x0f)) > 0x0f;
		fCarry = result > 0xff;
		A = result;
		break;
	case 0x81:
		result = A + C;
		fZero = ((result & 0xff) == 0);
		fSub = 0;
		fHalfCarry = ((A & 0x0f) + (C & 0x0f)) > 0x0f;
		fCarry = result > 0xff;
		A = result;
		break;
	case 0x82:
		result = A + D;
		fZero = ((result & 0xff) == 0);
		fSub = 0;
		fHalfCarry = ((A & 0x0f) + (D & 0x0f)) > 0x0f;
		fCarry = result > 0xff;
		A = result;
		break;
	case 0x83:
		result = A + E;
		fZero = ((result & 0xff) == 0);
		fSub = 0;
		fHalfCarry = ((A & 0x0f) + (E & 0x0f)) > 0x0f;
		fCarry = result > 0xff;
		A = result;
		break;
	case 0x84:
		result = A + H;
		fZero = ((result & 0xff) == 0);
		fSub = 0;
		fHalfCarry = ((A & 0x0f) + (H & 0x0f)) > 0x0f;
		fCarry = result > 0xff;
		A = result;
		break;
	case 0x85:
		result = A + L;
		fZero = ((result & 0xff) == 0);
		fSub = 0;
		fHalfCarry = ((A & 0x0f) + (L & 0x0f)) > 0x0f;
		fCarry = result > 0xff;
		A = result;
		break;
	case 0x86:
		temp = Read8(HL);
		result = A + temp;
		fZero = ((result & 0xff) == 0);
		fSub = 0;
		fHalfCarry = ((A & 0x0f) + (temp & 0x0f)) > 0x0f;
		fCarry = result > 0xff;
		A = result;
		break;
	case 0x87:
		result = A + A;
		fZero = ((result & 0xff) == 0);
		fSub = 0;
		fHalfCarry = ((A & 0x0f) + (A & 0x0f)) > 0x0f;
		fCarry = result > 0xff;
		A = result;
		break;
	case 0x88:
		result = A + B + fCarry;
		fSub = 0;
		fHalfCarry = ((A & 0x0F) + (B & 0x0f) + fCarry) > 0x0F;
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x89:
		result = A + C + fCarry;
		fSub = 0;
		fHalfCarry = ((A & 0x0F) + (C & 0x0f) + fCarry) > 0x0F;
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x8a:
		result = A + D + fCarry;
		fSub = 0;
		fHalfCarry = ((A & 0x0F) + (D & 0x0f) + fCarry) > 0x0F;
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x8b:
		result = A + E + fCarry;
		fSub = 0;
		fHalfCarry = ((A & 0x0F) + (E & 0x0f) + fCarry) > 0x0F;
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x8c:
		result = A + H + fCarry;
		fSub = 0;
		fHalfCarry = ((A & 0x0F) + (H & 0x0f) + fCarry) > 0x0F;
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x8d:
		result = A + L + fCarry;
		fSub = 0;
		fHalfCarry = ((A & 0x0F) + (L & 0x0f) + fCarry) > 0x0F;
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x8e:
		temp = Read8(HL);
		result = A + temp + fCarry;
		fSub = 0;
		fHalfCarry = ((A & 0x0F) + (temp & 0x0f) + fCarry) > 0x0F;
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x8f:
		result = A + A + fCarry;
		fSub = 0;
		fHalfCarry = ((A & 0x0F) + (A & 0x0f) + fCarry) > 0x0F;
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x90:
		result = A - B;
		fSub = 1;
		fHalfCarry = (B & 0x0f) > (A & 0x0f);
		fCarry = B > A;
		A = result;
		fZero = (A == 0);
		break;
	case 0x91:
		result = A - C;
		fSub = 1;
		fHalfCarry = (C & 0x0f) > (A & 0x0f);
		fCarry = C > A;
		A = result;
		fZero = (A == 0);
		break;
	case 0x92:
		result = A - D;
		fSub = 1;
		fHalfCarry = (D & 0x0f) > (A & 0x0f);
		fCarry = D > A;
		A = result;
		fZero = (A == 0);
		break;
	case 0x93:
		result = A - E;
		fSub = 1;
		fHalfCarry = (E & 0x0f) > (A & 0x0f);
		fCarry = E > A;
		A = result;
		fZero = (A == 0);
		break;
	case 0x94:
		result = A - H;
		fSub = 1;
		fHalfCarry = (H & 0x0f) > (A & 0x0f);
		fCarry = H > A;
		A = result;
		fZero = (A == 0);
		break;
	case 0x95:
		result = A - L;
		fSub = 1;
		fHalfCarry = (L & 0x0f) > (A & 0x0f);
		fCarry = L > A;
		A = result;
		fZero = (A == 0);
		break;
	case 0x96:
		temp = Read8(HL);
		result = A - temp;
		fSub = 1;
		fHalfCarry = ((uint8_t)temp & 0x0f) > (A & 0x0f);
		fCarry = temp > A;
		A = result;
		fZero = (A == 0);
		break;
	case 0x97:
		A = 0;
		fSub = 1;
		fCarry = 0;
		fHalfCarry = 0;
		fZero = 1;
		break;
	case 0x98:
		result = A - B - fCarry;
		fSub = 1;
		fHalfCarry = ((B & 0x0f) + fCarry) > (A & 0x0f);
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x99:
		result = A - C - fCarry;
		fSub = 1;
		fHalfCarry = ((C & 0x0f) + fCarry) > (A & 0x0f);
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x9a:
		result = A - D - fCarry;
		fSub = 1;
		fHalfCarry = ((D & 0x0f) + fCarry) > (A & 0x0f);
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x9b:
		result = A - E - fCarry;
		fSub = 1;
		fHalfCarry = ((E & 0x0f) + fCarry) > (A & 0x0f);
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x9c:
		result = A - H - fCarry;
		fSub = 1;
		fHalfCarry = ((H & 0x0f) + fCarry) > (A & 0x0f);
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x9d:
		result = A - L - fCarry;
		fSub = 1;
		fHalfCarry = ((L & 0x0f) + fCarry) > (A & 0x0f);
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x9e:
		temp = Read8(HL);
		result = A - temp - fCarry;
		fSub = 1;
		fHalfCarry = ((uint8_t)(temp & 0x0f) + fCarry) > (A & 0x0f);
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0x9f:
		result = A - A - fCarry;
		fSub = 1;
		fHalfCarry = ((A & 0x0f) + fCarry) > (A & 0x0f);
		A = result;
		fZero = (A == 0);
		break;
	case 0xa0:
		A &= B;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 1;
		fCarry = 0;
		break;
	case 0xa1:
		A &= C;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 1;
		fCarry = 0;
		break;
	case 0xa2:
		A &= D;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 1;
		fCarry = 0;
		break;
	case 0xa3:
		A &= E;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 1;
		fCarry = 0;
		break;
	case 0xa4:
		A &= H;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 1;
		fCarry = 0;
		break;
	case 0xa5:
		A &= L;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 1;
		fCarry = 0;
		break;
	case 0xa6:
		A &= Read8(HL);
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 1;
		fCarry = 0;
		break;
	case 0xa7:
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 1;
		fCarry = 0;
		break;
	case 0xa8:
		A ^= B;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xa9:
		A ^= C;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xaa:
		A ^= D;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xab:
		A ^= E;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xac:
		A ^= H;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xad:
		A ^= L;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xae:
		A ^= Read8(HL);
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xaf:
		A = 0;
		fZero = 1;
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xb0:
		A |= B;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xb1:
		A |= C;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xb2:
		A |= D;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xb3:
		A |= E;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xb4:
		A |= H;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xb5:
		A |= L;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xb6:
		A |= Read8(HL);
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xb7:
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xb8:
		fZero = (B == A);
		fSub = 1;
		fHalfCarry = (B & 0x0f) > (A & 0x0f);
		fCarry = B > A;
		break;
	case 0xb9:
		fZero = (C == A);
		fSub = 1;
		fHalfCarry = (C & 0x0f) > (A & 0x0f);
		fCarry = C > A;
		break;
	case 0xba:
		fZero = (D == A);
		fSub = 1;
		fHalfCarry = (D & 0x0f) > (A & 0x0f);
		fCarry = D > A;
		break;
	case 0xbb:
		fZero = (E == A);
		fSub = 1;
		fHalfCarry = (E & 0x0f) > (A & 0x0f);
		fCarry = E > A;
		break;
	case 0xbc:
		fZero = (H == A);
		fSub = 1;
		fHalfCarry = (H & 0x0f) > (A & 0x0f);
		fCarry = H > A;
		break;
	case 0xbd:
		fZero = (L == A);
		fSub = 1;
		fHalfCarry = (L & 0x0f) > (A & 0x0f);
		fCarry = L > A;
		break;
	case 0xbe:
		temp = Read8(HL);
		fZero = (temp == A);
		fSub = 1;
		fHalfCarry = ((uint8_t)temp & 0x0f) > (A & 0x0f);
		fCarry = temp > A;
		break;
	case 0xbf:
		fZero = 1;
		fSub = 1;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xc0:
		if (!fZero) {
			PC = Pop16();
			stepClock += 12;
		}
		break;
	case 0xc1:
		BC = Pop16();
		break;
	case 0xc2:
		if (!fZero) {
			PC = attr16;
			stepClock += 4;
		}
		break;
	case 0xc3:
		PC = attr16;
		break;
	case 0xc4:
		if (!fZero) {
			Push16(PC);
			PC = attr16;
			stepClock += 12;
		}
		break;
	case 0xc5:
		Push16(BC);
		break;
	case 0xc6:
		result = A + attr8;
		fSub = 0;
		fHalfCarry = ((A & 0x0f) + (attr8 & 0x0f)) > 0x0f;
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0xc7:
		Push16(PC);
		PC = 0x00;
		break;
	case 0xc8:
		if (fZero) {
			PC = Pop16();
			stepClock += 12;
		}
		break;
	case 0xc9:
		PC = Pop16();
		break;
	case 0xca:
		if (fZero) {
			PC = attr16;
			stepClock += 4;
		}
		break;
	case 0xcb:
		secondaryOpcode = Read8(PC);
		PC++;
		stepClock += prefixOpcodeTimingTable[secondaryOpcode] * 4;
		ExecutePrefixInline();
		break;
	case 0xcc:
		if (fZero) {
			Push16(PC);
			PC = attr16;
			stepClock += 12;
		}
		break;
	case 0xcd:
		Push16(PC);
		PC = attr16;
		break;
	case 0xce:
		result = A + attr8 + fCarry;
		fSub = 0;
		fHalfCarry = ((A & 0x0F) + (attr8 & 0x0f) + fCarry) > 0x0F;
		fCarry = result > 0xff;
		A = result;
		fZero = (A == 0);
		break;
	case 0xcf:
		Push16(PC);
		PC = 0x08;
		break;
	case 0xd0:
		if (!fCarry) {
			PC = Pop16();
			stepClock += 12;
		}
		break;
	case 0xd1:
		DE = Pop16();
		break;
	case 0xd2:
		if (!fCarry) {
			PC = attr16;
			stepClock += 4;
		}
		break;
	case 0xd4:
		if (!fCarry) {
			Push16(PC);
			PC = attr16;
			stepClock += 12;
		}
		break;
	case 0xd5:
		Push16(DE);
		break;
	case 0xd6:
		result = A - attr8;
		fSub = 1;
		fHalfCarry = (attr8 & 0x0f) > (A & 0x0f);
		fCarry = attr8 > A;
		A = result;
		fZero = (A == 0);
		break;
	case 0xd7:
		Push16(PC);
		PC = 0x10;
		break;
	case 0xd8:
		if (fCarry) {
			PC = Pop16();
			stepClock += 12;
		}
		break;
	case 0xd9:
		PC = Pop16();
		IME = 1;
		break;
	case 0xda:
		if (fCarry) {
			PC = attr16;
			stepClock += 4;
		}
		break;
	case 0xdc:
		if (fCarry) {
			Push16(PC);
			PC = attr16;
			stepClock += 12;
		}
		break;
	case 0xde:
		result = A - attr8 - fCarry;
		fSub = 1;
		fHalfCarry = ((attr8 & 0x0f) + fCarry) > (A & 0x0f);
		fCarry = (attr8 + fCarry) > A;
		A = result;
		fZero = (A == 0);
		break;
	case 0xdf:
		Push16(PC);
		PC = 0x18;
		break;
	case 0xe0:
		Write8(0xff00 | attr8, A);
		break;
	case 0xe1:
		HL = Pop16();
		break;
	case 0xe2:
		Write8(0xff00 | C, A);
		break;
	case 0xe5:
		Push16(HL);
		break;
	case 0xe6:
		A &= attr8;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 1;
		fCarry = 0;
		break;
	case 0xe7:
		Push16(PC);
		PC = 0x20;
		break;
	case 0xe8:
		result = SP + (int8_t)attr8;
		fHalfCarry = ((SP ^ attr8 ^ result) & 0x10) == 0x10;
		fCarry = ((SP ^ (int16_t)((int8_t)attr8) ^ result) & 0x100) == 0x100;
		fZero = 0;
		fSub = 0;
		SP = result;
		break;
	case 0xe9:
		PC = HL;
		break;
	case 0xea:
		Write8(attr16, A);
		break;
	case 0xee:
		A ^= attr8;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xef:
		Push16(PC);
		PC = 0x28;
		break;
	case 0xf0:
		A = Read8(0xff00 | attr8);
		break;
	case 0xf1:
		AF = Pop16() & 0xfff0;
		break;
	case 0xf2:
		A = Read8(0xff00 | C);
		break;
	case 0xf3:
		IME = 0;
		break;
	case 0xf5:
		Push16(AF & 0xfff0);
		break;
	case 0xf6:
		A |= attr8;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0xf7:
		Push16(PC);
		PC = 0x30;
		break;
	case 0xf8:
		result = SP + (int8_t)attr8;
		fZero = 0;
		fSub = 0;
		fHalfCarry = ((SP ^ attr8 ^ result) & 0x10) == 0x10;
		fCarry = ((SP ^ (int16_t)((int8_t)attr8) ^ result) & 0x100) == 0x100;
		HL = result;
		break;
	case 0xf9:
		SP = HL;
		break;
	case 0xfa:
		A = Read8(attr16);
		break;
	case 0xfb:
		//stepClock += Step();
		//if (opcode != 0xf3) {
			IME = 1;
		//}
		break;
	case 0xfe:
		fZero = (A == attr8);
		fSub = 1;
		fHalfCarry = (attr8 & 0x0f) > (A & 0x0f);
		fCarry = attr8 > A;
		break;
	case 0xff:
		Push16(PC);
		PC = 0x38;
		break;
	default:
		std::cout << "Unhandled instruction: 0x" << std::hex << (uint32_t)opcode << "\n";
		ExecuteThroughTable();
	}
}

void CPU::ExecutePrefixThroughTable() {
	(this->*prefixOpcodeTable[secondaryOpcode].inst)();
}

void CPU::ExecutePrefixInline() {
	uint8_t carry, sign;
	uint32_t temp;
	switch (secondaryOpcode) {
	case 0x00:
		carry = B >> 7;
		B = (B << 1) | carry;
		fZero = (B == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x01:
		carry = C >> 7;
		C = (C << 1) | carry;
		fZero = (C == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x02:
		carry = D >> 7;
		D = (D << 1) | carry;
		fZero = (D == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x03:
		carry = E >> 7;
		E = (E << 1) | carry;
		fZero = (E == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x04:
		carry = H >> 7;
		H = (H << 1) | carry;
		fZero = (H == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x05:
		carry = L >> 7;
		L = (L << 1) | carry;
		fZero = (L == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x06:
		temp = Read8(HL);
		carry = temp >> 7;
		temp = (temp << 1) | carry;
		Write8(HL, temp);
		fZero = (temp == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x07:
		carry = A >> 7;
		A = (A << 1) | carry;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x08:
		carry = B & 0b1;
		B = (B >> 1) | (carry << 7);
		fZero = (B == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x09:
		carry = C & 0b1;
		C = (C >> 1) | (carry << 7);
		fZero = (C == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x0a:
		carry = D & 0b1;
		D = (D >> 1) | (carry << 7);
		fZero = (D == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x0b:
		carry = E & 0b1;
		E = (E >> 1) | (carry << 7);
		fZero = (E == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x0c:
		carry = H & 0b1;
		H = (H >> 1) | (carry << 7);
		fZero = (H == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x0d:
		carry = L & 0b1;
		L = (L >> 1) | (carry << 7);
		fZero = (L == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x0e:
		temp = Read8(HL);
		carry = temp & 0b1;
		temp = (temp >> 1) | (carry << 7);
		Write8(HL, temp);
		fZero = (temp == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x0f:
		carry = A & 0b1;
		A = (A >> 1) | (carry << 7);
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x10:
		carry = B >> 7;
		B = (B << 1) | fCarry;
		fZero = (B == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x11:
		carry = C >> 7;
		C = (C << 1) | fCarry;
		fZero = (C == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x12:
		carry = D >> 7;
		D = (D << 1) | fCarry;
		fZero = (D == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x13:
		carry = E >> 7;
		E = (E << 1) | fCarry;
		fZero = (E == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x14:
		carry = H >> 7;
		H = (H << 1) | fCarry;
		fZero = (H == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x15:
		carry = L >> 7;
		L = (L << 1) | fCarry;
		fZero = (L == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x16:
		temp = Read8(HL);
		carry = temp >> 7;
		temp = (temp << 1) | fCarry;
		Write8(HL, temp);
		fZero = ((temp & 0xff) == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x17:
		carry = A >> 7;
		A = (A << 1) | fCarry;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x18:
		carry = B & 0b1;
		B = (B >> 1) | (fCarry << 7);
		fZero = (B == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x19:
		carry = C & 0b1;
		C = (C >> 1) | (fCarry << 7);
		fZero = (C == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x1a:
		carry = D & 0b1;
		D = (D >> 1) | (fCarry << 7);
		fZero = (D == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x1b:
		carry = E & 0b1;
		E = (E >> 1) | (fCarry << 7);
		fZero = (E == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x1c:
		carry = H & 0b1;
		H = (H >> 1) | (fCarry << 7);
		fZero = (H == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x1d:
		carry = L & 0b1;
		L = (L >> 1) | (fCarry << 7);
		fZero = (L == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x1e:
		temp = Read8(HL);
		carry = temp & 0b1;
		temp = (temp >> 1) | (fCarry << 7);
		fZero = (temp == 0);
		Write8(HL, temp);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x1f:
		carry = A & 0b1;
		A = (A >> 1) | (fCarry << 7);
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x20:
		carry = B >> 7;
		B = (B << 1);
		fZero = (B == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x21:
		carry = C >> 7;
		C = (C << 1);
		fZero = (C == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x22:
		carry = D >> 7;
		D = (D << 1);
		fZero = (D == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x23:
		carry = E >> 7;
		E = (E << 1);
		fZero = (E == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x24:
		carry = H >> 7;
		H = (H << 1);
		fZero = (H == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x25:
		carry = L >> 7;
		L = (L << 1);
		fZero = (L == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x26:
		temp = Read8(HL);
		carry = temp >> 7;
		temp = (temp << 1);
		Write8(HL, temp);
		fZero = ((temp & 0xff) == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x27:
		carry = A >> 7;
		A = (A << 1);
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x28:
		carry = B & 0b1;
		sign = B & (0b1 << 7);
		B = (B >> 1) | sign;
		fZero = (B == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x29:
		carry = C & 0b1;
		sign = C & (0b1 << 7);
		C = (C >> 1) | sign;
		fZero = (C == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x2a:
		carry = D & 0b1;
		sign = D & (0b1 << 7);
		D = (D >> 1) | sign;
		fZero = (D == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x2b:
		carry = E & 0b1;
		sign = E & (0b1 << 7);
		E = (E >> 1) | sign;
		fZero = (E == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x2c:
		carry = H & 0b1;
		sign = H & (0b1 << 7);
		H = (H >> 1) | sign;
		fZero = (H == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x2d:
		carry = L & 0b1;
		sign = L & (0b1 << 7);
		L = (L >> 1) | sign;
		fZero = (L == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x2e:
		temp = Read8(HL);
		carry = temp & 0b1;
		sign = temp & (0b1 << 7);
		temp = (temp >> 1) | sign;
		Write8(HL, temp);
		fZero = (temp == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x2f:
		carry = A & 0b1;
		sign = A & (0b1 << 7);
		A = (A >> 1) | sign;
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x30:
		B = (B >> 4) | ((B << 4) & 0xf0);
		fZero = (B == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0x31:
		C = (C >> 4) | ((C << 4) & 0xf0);
		fZero = (C == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0x32:
		D = (D >> 4) | ((D << 4) & 0xf0);
		fZero = (D == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0x33:
		E = (E >> 4) | ((E << 4) & 0xf0);
		fZero = (E == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0x34:
		H = (H >> 4) | ((H << 4) & 0xf0);
		fZero = (H == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0x35:
		L = (L >> 4) | ((L << 4) & 0xf0);
		fZero = (L == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0x36:
		temp = Read8(HL);
		temp = (temp >> 4) | ((temp << 4) & 0xf0);
		Write8(HL, temp);
		fZero = (temp == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0x37:
		A = (A >> 4) | ((A << 4) & 0xf0);
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = 0;
		break;
	case 0x38:
		carry = B & 0b1;
		B = (B >> 1);
		fZero = (B == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x39:
		carry = C & 0b1;
		C = (C >> 1);
		fZero = (C == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x3a:
		carry = D & 0b1;
		D = (D >> 1);
		fZero = (D == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x3b:
		carry = E & 0b1;
		E = (E >> 1);
		fZero = (E == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x3c:
		carry = H & 0b1;
		H = (H >> 1);
		fZero = (H == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x3d:
		carry = L & 0b1;
		L = (L >> 1);
		fZero = (L == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x3e:
		temp = Read8(HL);
		carry = temp & 0b1;
		temp = (temp >> 1);
		Write8(HL, temp);
		fZero = (temp == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x3f:
		carry = A & 0b1;
		A = (A >> 1);
		fZero = (A == 0);
		fSub = 0;
		fHalfCarry = 0;
		fCarry = carry;
		break;
	case 0x40:
		fZero = ((B >> 0) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x41:
		fZero = ((C >> 0) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x42:
		fZero = ((D >> 0) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x43:
		fZero = ((E >> 0) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x44:
		fZero = ((H >> 0) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x45:
		fZero = ((L >> 0) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x46:
		fZero = ((Read8(HL) >> 0) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x47:
		fZero = ((A >> 0) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x48:
		fZero = ((B >> 1) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x49:
		fZero = ((C >> 1) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x4a:
		fZero = ((D >> 1) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x4b:
		fZero = ((E >> 1) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x4c:
		fZero = ((H >> 1) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x4d:
		fZero = ((L >> 1) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x4e:
		fZero = ((Read8(HL) >> 1) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x4f:
		fZero = ((A >> 1) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x50:
		fZero = ((B >> 2) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x51:
		fZero = ((C >> 2) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x52:
		fZero = ((D >> 2) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x53:
		fZero = ((E >> 2) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x54:
		fZero = ((H >> 2) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x55:
		fZero = ((L >> 2) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x56:
		fZero = ((Read8(HL) >> 2) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x57:
		fZero = ((A >> 2) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x58:
		fZero = ((B >> 3) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x59:
		fZero = ((C >> 3) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x5a:
		fZero = ((D >> 3) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x5b:
		fZero = ((E >> 3) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x5c:
		fZero = ((H >> 3) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x5d:
		fZero = ((L >> 3) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x5e:
		fZero = ((Read8(HL) >> 3) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x5f:
		fZero = ((A >> 3) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x60:
		fZero = ((B >> 4) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x61:
		fZero = ((C >> 4) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x62:
		fZero = ((D >> 4) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x63:
		fZero = ((E >> 4) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x64:
		fZero = ((H >> 4) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x65:
		fZero = ((L >> 4) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x66:
		fZero = ((Read8(HL) >> 4) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x67:
		fZero = ((A >> 4) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x68:
		fZero = ((B >> 5) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x69:
		fZero = ((C >> 5) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x6a:
		fZero = ((D >> 5) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x6b:
		fZero = ((E >> 5) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x6c:
		fZero = ((H >> 5) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x6d:
		fZero = ((L >> 5) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x6e:
		fZero = ((Read8(HL) >> 5) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x6f:
		fZero = ((A >> 5) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x70:
		fZero = ((B >> 6) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x71:
		fZero = ((C >> 6) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x72:
		fZero = ((D >> 6) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x73:
		fZero = ((E >> 6) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x74:
		fZero = ((H >> 6) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x75:
		fZero = ((L >> 6) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x76:
		fZero = ((Read8(HL) >> 6) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x77:
		fZero = ((A >> 6) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x78:
		fZero = ((B >> 7) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x79:
		fZero = ((C >> 7) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x7a:
		fZero = ((D >> 7) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x7b:
		fZero = ((E >> 7) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x7c:
		fZero = ((H >> 7) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x7d:
		fZero = ((L >> 7) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x7e:
		fZero = ((Read8(HL) >> 7) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x7f:
		fZero = ((A >> 7) & 0b1) == 0;
		fSub = 0;
		fHalfCarry = 1;
		break;
	case 0x80:
		B &= ~(1 << 0);
		break;
	case 0x81:
		C &= ~(1 << 0);
		break;
	case 0x82:
		D &= ~(1 << 0);
		break;
	case 0x83:
		E &= ~(1 << 0);
		break;
	case 0x84:
		H &= ~(1 << 0);
		break;
	case 0x85:
		L &= ~(1 << 0);
		break;
	case 0x86:
		Write8(HL, Read8(HL) & ~(1 << 0));
		break;
	case 0x87:
		A &= ~(1 << 0);
		break;
	case 0x88:
		B &= ~(1 << 1);
		break;
	case 0x89:
		C &= ~(1 << 1);
		break;
	case 0x8a:
		D &= ~(1 << 1);
		break;
	case 0x8b:
		E &= ~(1 << 1);
		break;
	case 0x8c:
		H &= ~(1 << 1);
		break;
	case 0x8d:
		L &= ~(1 << 1);
		break;
	case 0x8e:
		Write8(HL, Read8(HL) & ~(1 << 1));
		break;
	case 0x8f:
		A &= ~(1 << 1);
		break;
	case 0x90:
		B &= ~(1 << 2);
		break;
	case 0x91:
		C &= ~(1 << 2);
		break;
	case 0x92:
		D &= ~(1 << 2);
		break;
	case 0x93:
		E &= ~(1 << 2);
		break;
	case 0x94:
		H &= ~(1 << 2);
		break;
	case 0x95:
		L &= ~(1 << 2);
		break;
	case 0x96:
		Write8(HL, Read8(HL) & ~(1 << 2));
		break;
	case 0x97:
		A &= ~(1 << 2);
		break;
	case 0x98:
		B &= ~(1 << 3);
		break;
	case 0x99:
		C &= ~(1 << 3);
		break;
	case 0x9a:
		D &= ~(1 << 3);
		break;
	case 0x9b:
		E &= ~(1 << 3);
		break;
	case 0x9c:
		H &= ~(1 << 3);
		break;
	case 0x9d:
		L &= ~(1 << 3);
		break;
	case 0x9e:
		Write8(HL, Read8(HL) & ~(1 << 3));
		break;
	case 0x9f:
		A &= ~(1 << 3);
		break;
	case 0xa0:
		B &= ~(1 << 4);
		break;
	case 0xa1:
		C &= ~(1 << 4);
		break;
	case 0xa2:
		D &= ~(1 << 4);
		break;
	case 0xa3:
		E &= ~(1 << 4);
		break;
	case 0xa4:
		H &= ~(1 << 4);
		break;
	case 0xa5:
		L &= ~(1 << 4);
		break;
	case 0xa6:
		Write8(HL, Read8(HL) & ~(1 << 4));
		break;
	case 0xa7:
		A &= ~(1 << 4);
		break;
	case 0xa8:
		B &= ~(1 << 5);
		break;
	case 0xa9:
		C &= ~(1 << 5);
		break;
	case 0xaa:
		D &= ~(1 << 5);
		break;
	case 0xab:
		E &= ~(1 << 5);
		break;
	case 0xac:
		H &= ~(1 << 5);
		break;
	case 0xad:
		L &= ~(1 << 5);
		break;
	case 0xae:
		Write8(HL, Read8(HL) & ~(1 << 5));
		break;
	case 0xaf:
		A &= ~(1 << 5);
		break;
	case 0xb0:
		B &= ~(1 << 6);
		break;
	case 0xb1:
		C &= ~(1 << 6);
		break;
	case 0xb2:
		D &= ~(1 << 6);
		break;
	case 0xb3:
		E &= ~(1 << 6);
		break;
	case 0xb4:
		H &= ~(1 << 6);
		break;
	case 0xb5:
		L &= ~(1 << 6);
		break;
	case 0xb6:
		Write8(HL, Read8(HL) & ~(1 << 6));
		break;
	case 0xb7:
		A &= ~(1 << 6);
		break;
	case 0xb8:
		B &= ~(1 << 7);
		break;
	case 0xb9:
		C &= ~(1 << 7);
		break;
	case 0xba:
		D &= ~(1 << 7);
		break;
	case 0xbb:
		E &= ~(1 << 7);
		break;
	case 0xbc:
		H &= ~(1 << 7);
		break;
	case 0xbd:
		L &= ~(1 << 7);
		break;
	case 0xbe:
		Write8(HL, Read8(HL) & ~(1 << 7));
		break;
	case 0xbf:
		A &= ~(1 << 7);
		break;
	case 0xc0:
		B |= (1 << 0);
		break;
	case 0xc1:
		C |= (1 << 0);
		break;
	case 0xc2:
		D |= (1 << 0);
		break;
	case 0xc3:
		E |= (1 << 0);
		break;
	case 0xc4:
		H |= (1 << 0);
		break;
	case 0xc5:
		L |= (1 << 0);
		break;
	case 0xc6:
		Write8(HL, Read8(HL) | (1 << 0));
		break;
	case 0xc7:
		A |= (1 << 0);
		break;
	case 0xc8:
		B |= (1 << 1);
		break;
	case 0xc9:
		C |= (1 << 1);
		break;
	case 0xca:
		D |= (1 << 1);
		break;
	case 0xcb:
		E |= (1 << 1);
		break;
	case 0xcc:
		H |= (1 << 1);
		break;
	case 0xcd:
		L |= (1 << 1);
		break;
	case 0xce:
		Write8(HL, Read8(HL) | (1 << 1));
		break;
	case 0xcf:
		A |= (1 << 1);
		break;
	case 0xd0:
		B |= (1 << 2);
		break;
	case 0xd1:
		C |= (1 << 2);
		break;
	case 0xd2:
		D |= (1 << 2);
		break;
	case 0xd3:
		E |= (1 << 2);
		break;
	case 0xd4:
		H |= (1 << 2);
		break;
	case 0xd5:
		L |= (1 << 2);
		break;
	case 0xd6:
		Write8(HL, Read8(HL) | (1 << 2));
		break;
	case 0xd7:
		A |= (1 << 2);
		break;
	case 0xd8:
		B |= (1 << 3);
		break;
	case 0xd9:
		C |= (1 << 3);
		break;
	case 0xda:
		D |= (1 << 3);
		break;
	case 0xdb:
		E |= (1 << 3);
		break;
	case 0xdc:
		H |= (1 << 3);
		break;
	case 0xdd:
		L |= (1 << 3);
		break;
	case 0xde:
		Write8(HL, Read8(HL) | (1 << 3));
		break;
	case 0xdf:
		A |= (1 << 3);
		break;
	case 0xe0:
		B |= (1 << 4);
		break;
	case 0xe1:
		C |= (1 << 4);
		break;
	case 0xe2:
		D |= (1 << 4);
		break;
	case 0xe3:
		E |= (1 << 4);
		break;
	case 0xe4:
		H |= (1 << 4);
		break;
	case 0xe5:
		L |= (1 << 4);
		break;
	case 0xe6:
		Write8(HL, Read8(HL) | (1 << 4));
		break;
	case 0xe7:
		A |= (1 << 4);
		break;
	case 0xe8:
		B |= (1 << 5);
		break;
	case 0xe9:
		C |= (1 << 5);
		break;
	case 0xea:
		D |= (1 << 5);
		break;
	case 0xeb:
		E |= (1 << 5);
		break;
	case 0xec:
		H |= (1 << 5);
		break;
	case 0xed:
		L |= (1 << 5);
		break;
	case 0xee:
		Write8(HL, Read8(HL) | (1 << 5));
		break;
	case 0xef:
		A |= (1 << 5);
		break;
	case 0xf0:
		B |= (1 << 6);
		break;
	case 0xf1:
		C |= (1 << 6);
		break;
	case 0xf2:
		D |= (1 << 6);
		break;
	case 0xf3:
		E |= (1 << 6);
		break;
	case 0xf4:
		H |= (1 << 6);
		break;
	case 0xf5:
		L |= (1 << 6);
		break;
	case 0xf6:
		Write8(HL, Read8(HL) | (1 << 6));
		break;
	case 0xf7:
		A |= (1 << 6);
		break;
	case 0xf8:
		B |= (1 << 7);
		break;
	case 0xf9:
		C |= (1 << 7);
		break;
	case 0xfa:
		D |= (1 << 7);
		break;
	case 0xfb:
		E |= (1 << 7);
		break;
	case 0xfc:
		H |= (1 << 7);
		break;
	case 0xfd:
		L |= (1 << 7);
		break;
	case 0xfe:
		Write8(HL, Read8(HL) | (1 << 7));
		break;
	case 0xff:
		A |= (1 << 7);
		break;
	default:
		std::cout << "Unhandled CPU instruction. Will execute through table: 0x" << std::hex << opcode << std::dec << "\n";
		ExecutePrefixThroughTable();
	}
}

// Custom Instruction
void CPU::UNH() {
	std::cout << "Unhandled CPU instruction: 0x" << std::hex << opcode << std::dec << "\n";
}

void CPU::PREFIX() {
	secondaryOpcode = Read8(PC);
	PC++;
	stepClock += prefixOpcodeTimingTable[secondaryOpcode] * 4;
	ExecutePrefixThroughTable();
}

// 8-Bit Arithmetic and Logic Instructions
void CPU::ADC_A_R8() {
	uint8_t rs = opcode & 0b111;
	uint16_t temp = A;
	uint16_t value = GetR8(rs);
	uint16_t result = temp + value + fCarry;
	A = (result & 0xff);
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = ((temp & 0x0F) + (value & 0x0f) + fCarry) > 0x0F;
	fCarry = result > 0xff;
}

void CPU::ADC_A_HL() {
	uint16_t temp = A;
	uint16_t value = Read8(HL);
	uint16_t result = temp + value + fCarry;
	A = (result & 0xff);
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = ((temp & 0x0F) + (value & 0x0f) + fCarry) > 0x0F;
	fCarry = result > 0xff;
}

void CPU::ADC_A_N8() {
	uint16_t temp = A;
	uint16_t value = attr8;
	uint16_t result = temp + value + fCarry;
	A = (result & 0xff);
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = ((temp & 0x0F) + (value & 0x0f) + fCarry) > 0x0F;
	fCarry = result > 0xff;
}

void CPU::ADD_A_R8() {
	uint8_t rs = opcode & 0b111;
	uint16_t temp = A;
	uint16_t value = GetR8(rs);
	uint16_t result = temp + value;
	A = result & 0xff;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = ((temp & 0x0f) + (value & 0x0f)) > 0x0f;
	fCarry = result > 0xff;
}

void CPU::ADD_A_HL() {
	uint16_t temp = A;
	uint16_t value = Read8(HL);
	uint16_t result = temp + value;
	A = result & 0xff;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = ((temp & 0x0f) + (value & 0x0f)) > 0x0f;
	fCarry = result > 0xff;
}

void CPU::ADD_A_N8() {
	uint16_t temp = A;
	uint16_t value = attr8;
	uint16_t result = temp + value;
	A = result & 0xff;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = ((temp & 0x0f) + (value & 0x0f)) > 0x0f;
	fCarry = result > 0xff;
}

void CPU::AND_A_R8() {
	uint8_t rs = opcode & 0b111;
	uint8_t value = GetR8(rs);
	A &= value;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = 1;
	fCarry = 0;
}

void CPU::AND_A_HL() {
	uint8_t value = Read8(HL);
	A &= value;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = 1;
	fCarry = 0;
}

void CPU::AND_A_N8() {
	A &= attr8;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = 1;
	fCarry = 0;
}

void CPU::CP_A_R8() {
	uint8_t rs = opcode & 0b111;
	uint16_t temp = A;
	uint16_t value = GetR8(rs);
	fZero = (temp == value);
	fSub = 1;
	fHalfCarry = (value & 0x0f) > (temp & 0x0f);
	fCarry = value > temp;
}

void CPU::CP_A_HL() {
	uint16_t temp = A;
	uint16_t value = Read8(HL);
	fZero = (temp == value);
	fSub = 1;
	fHalfCarry = (value & 0x0f) > (temp & 0x0f);
	fCarry = value > temp;
}

void CPU::CP_A_N8() {
	uint16_t temp = A;
	uint16_t value = attr8;
	fZero = (temp == value);
	fSub = 1;
	fHalfCarry = (value & 0x0f) > (temp & 0x0f);
	fCarry = value > temp;
}

void CPU::DEC_R8() {
	uint8_t rd = (opcode >> 3) & 0b111;
	uint8_t temp = GetR8(rd);
	uint8_t result = temp - 1;
	SetR8(rd, result);
	fZero = (result == 0);
	fSub = 1;
	fHalfCarry = (temp & 0xf) == 0;
}

void CPU::DEC_HL() {
	uint8_t temp = Read8(HL);
	uint8_t result = temp - 1;
	Write8(HL, result);
	fZero = (result == 0);
	fSub = 1;
	fHalfCarry = (temp & 0xf) == 0;
}

void CPU::INC_R8() {
	uint8_t rd = (opcode >> 3) & 0b111;
	uint8_t temp = GetR8(rd);
	uint8_t result = temp + 1;
	SetR8(rd, result);
	fZero = (result == 0);
	fSub = 0;
	fHalfCarry = (temp & 0xf) == 0xf;
}

void CPU::INC_HL() {
	uint8_t temp = Read8(HL);
	uint8_t result = temp + 1;
	Write8(HL, result);
	fZero = (result == 0);
	fSub = 0;
	fHalfCarry = (temp & 0xf) == 0xf;
}

void CPU::OR_A_R8() {
	uint8_t rs = opcode & 0b111;
	uint8_t value = GetR8(rs);
	A |= value;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = 0;
}

void CPU::OR_A_HL() {
	uint8_t value = Read8(HL);
	A |= value;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = 0;
}

void CPU::OR_A_N8() {
	A |= attr8;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = 0;
}

void CPU::SBC_A_R8() {
	uint8_t rs = opcode & 0b111;
	uint16_t temp = A;
	uint16_t value = GetR8(rs);
	uint16_t result = temp - value - fCarry;
	A = result & 0xff;
	fZero = (A == 0);
	fSub = 1;
	fHalfCarry = ((value & 0x0f) + fCarry) > (temp & 0x0f);
	fCarry = (value + fCarry) > temp;
}

void CPU::SBC_A_HL() {
	uint16_t temp = A;
	uint16_t value = Read8(HL);
	uint16_t result = temp - value - fCarry;
	A = result & 0xff;
	fZero = (A == 0);
	fSub = 1;
	fHalfCarry = ((value & 0x0f) + fCarry) > (temp & 0x0f);
	fCarry = (value + fCarry) > temp;
}

void CPU::SBC_A_N8() {
	uint16_t temp = A;
	uint16_t value = attr8;
	uint16_t result = temp - value - fCarry;
	A = result & 0xff;
	fZero = (A == 0);
	fSub = 1;
	fHalfCarry = ((value & 0x0f) + fCarry) > (temp & 0x0f);
	fCarry = (value + fCarry) > temp;
}

void CPU::SUB_A_R8() {
	uint8_t rs = opcode & 0b111;
	uint16_t temp = A;
	uint16_t value = GetR8(rs);
	uint16_t result = temp - value;
	A = result & 0xff;
	fZero = (A == 0);
	fSub = 1;
	fHalfCarry = (value & 0x0f) > (temp & 0x0f);
	fCarry = value > temp;
}

void CPU::SUB_A_HL() {
	uint16_t temp = A;
	uint16_t value = Read8(HL);
	uint16_t result = temp - value;
	A = result & 0xff;
	fZero = (A == 0);
	fSub = 1;
	fHalfCarry = (value & 0x0f) > (temp & 0x0f);
	fCarry = value > temp;
}

void CPU::SUB_A_N8() {
	uint16_t temp = A;
	uint16_t value = attr8;
	uint16_t result = temp - value;
	A = result & 0xff;
	fZero = (A == 0);
	fSub = 1;
	fHalfCarry = (value & 0x0f) > (temp & 0x0f);
	fCarry = value > temp;
}

void CPU::XOR_A_R8() {
	uint8_t rs = opcode & 0b111;
	uint8_t value = GetR8(rs);
	A ^= value;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = 0;
}

void CPU::XOR_A_HL() {
	uint8_t value = Read8(HL);
	A ^= value;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = 0;
}

void CPU::XOR_A_N8() {
	A ^= attr8;
	fZero = (A == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = 0;
}

// 16-Bit Arithmetic Instructions
void CPU::ADD_HL_R16() {
	uint8_t rs = (opcode >> 4) & 0b11;
	uint32_t value = GetR16(rs);
	uint32_t temp = HL;
	uint32_t result = temp + value;
	HL = result;
	fSub = 0;
	fHalfCarry = ((temp & 0x0fff) + (value & 0x0fff)) > 0x0fff;
	fCarry = result > 0xffff;
}

void CPU::DEC_R16() {
	uint8_t rd = (opcode >> 4) & 0b11;
	uint16_t value = GetR16(rd);
	SetR16(rd, value - 1);
}

void CPU::INC_R16() {
	uint8_t rd = (opcode >> 4) & 0b11;
	uint16_t value = GetR16(rd);
	SetR16(rd, value + 1);
}

// Bit Operations Instructions
void CPU::BIT_U3_R8() {
	uint8_t rs = secondaryOpcode & 0b111;
	uint8_t bit = (secondaryOpcode >> 3) & 0b111;
	uint8_t value = GetR8(rs);
	fZero = ((value >> bit) & 0b1) == 0;
	fSub = 0;
	fHalfCarry = 1;
}

void CPU::BIT_U3_HL() {
	uint8_t bit = (secondaryOpcode >> 3) & 0b111;
	uint8_t value = Read8(HL);
	fZero = ((value >> bit) & 0b1) == 0;
	fSub = 0;
	fHalfCarry = 1;
}

void CPU::RES_U3_R8() {
	uint8_t rd = secondaryOpcode & 0b111;
	uint8_t bit = (secondaryOpcode >> 3) & 0b111;
	uint8_t value = GetR8(rd);
	SetR8(rd, value & (~(1 << bit)));
}

void CPU::RES_U3_HL() {
	uint8_t bit = (secondaryOpcode >> 3) & 0b111;
	uint8_t value = Read8(HL);
	Write8(HL, value & (~(1 << bit)));
}

void CPU::SET_U3_R8() {
	uint8_t rd = secondaryOpcode & 0b111;
	uint8_t bit = (secondaryOpcode >> 3) & 0b111;
	uint8_t value = GetR8(rd);
	SetR8(rd, value | (1 << bit));
}

void CPU::SET_U3_HL() {
	uint8_t bit = (secondaryOpcode >> 3) & 0b111;
	uint8_t value = Read8(HL);
	Write8(HL, value | (1 << bit));
}

void CPU::SWAP_R8() {
	uint8_t rd = secondaryOpcode & 0b111;
	uint8_t value = GetR8(rd);
	SetR8(rd, (value >> 4) | ((value << 4) & 0xf0));
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = 0;
}

void CPU::SWAP_HL() {
	uint8_t value = Read8(HL);
	Write8(HL, (value >> 4) | ((value << 4) & 0xf0));
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = 0;
}

// Bit Shift Instructions
void CPU::RL_R8() {
	uint8_t rd = secondaryOpcode & 0b111;
	uint8_t value = GetR8(rd);
	uint8_t carry = value >> 7;
	value = (value << 1) | fCarry;
	SetR8(rd, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::RL_HL() {
	uint8_t value = Read8(HL);
	uint8_t carry = value >> 7;
	value = (value << 1) | fCarry;
	Write8(HL, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::RL_A() {
	uint8_t carry = A >> 7;
	A = (A << 1) | fCarry;
	fZero = 0;
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::RLC_R8() {
	uint8_t rd = secondaryOpcode & 0b111;
	uint8_t value = GetR8(rd);
	uint8_t carry = value >> 7;
	value = (value << 1) | carry;
	SetR8(rd, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::RLC_HL() {
	uint8_t value = Read8(HL);
	uint8_t carry = value >> 7;
	value = (value << 1) | carry;
	Write8(HL, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::RLC_A() {
	uint8_t carry = A >> 7;
	A = (A << 1) | carry;
	fZero = 0;
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::RR_R8() {
	uint8_t rd = secondaryOpcode & 0b111;
	uint8_t value = GetR8(rd);
	uint8_t carry = value & 0b1;
	value = (value >> 1) | (fCarry << 7);
	SetR8(rd, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::RR_HL() {
	uint8_t value = Read8(HL);
	uint8_t carry = value & 0b1;
	value = (value >> 1) | (fCarry << 7);
	Write8(HL, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::RR_A() {
	uint8_t carry = A & 0b1;
	A = (A >> 1) | (fCarry << 7);
	fZero = 0;
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::RRC_R8() {
	uint8_t rd = secondaryOpcode & 0b111;
	uint8_t value = GetR8(rd);
	uint8_t carry = value & 0b1;
	value = (value >> 1) | (carry << 7);
	SetR8(rd, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::RRC_HL() {
	uint8_t value = Read8(HL);
	uint8_t carry = value & 0b1;
	value = (value >> 1) | (carry << 7);
	Write8(HL, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::RRC_A() {
	uint8_t carry = A & 0b1;
	A = (A >> 1) | (carry << 7);
	fZero = 0;
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::SLA_R8() {
	uint8_t rd = secondaryOpcode & 0b111;
	uint8_t value = GetR8(rd);
	uint8_t carry = value >> 7;
	value = (value << 1);
	SetR8(rd, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::SLA_HL() {
	uint8_t value = Read8(HL);
	uint8_t carry = value >> 7;
	value = (value << 1);
	Write8(HL, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::SRA_R8() {
	uint8_t rd = secondaryOpcode & 0b111;
	uint8_t value = GetR8(rd);
	uint8_t carry = value & 0b1;
	uint8_t sign = value & (0b1 << 7);
	value = (value >> 1) | sign;
	SetR8(rd, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::SRA_HL() {
	uint8_t value = Read8(HL);
	uint8_t carry = value & 0b1;
	uint8_t sign = value & (0b1 << 7);
	value = (value >> 1) | sign;
	Write8(HL, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::SRL_R8() {
	uint8_t rd = secondaryOpcode & 0b111;
	uint8_t value = GetR8(rd);
	uint8_t carry = value & 0b1;
	value = (value >> 1);
	SetR8(rd, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

void CPU::SRL_HL() {
	uint8_t value = Read8(HL);
	uint8_t carry = value & 0b1;
	value = (value >> 1);
	Write8(HL, value);
	fZero = (value == 0);
	fSub = 0;
	fHalfCarry = 0;
	fCarry = carry;
}

// Load Instructions
void CPU::LD_R8_R8() {
	uint8_t rs = opcode & 0b111;
	uint8_t rd = (opcode >> 3) & 0b111;
	uint8_t value = GetR8(rs);
	SetR8(rd, value);
}

void CPU::LD_R8_N8() {
	uint8_t rd = (opcode >> 3) & 0b111;
	SetR8(rd, attr8);
}

void CPU::LD_R16_N16() {
	uint8_t rd = (opcode >> 4) & 0b11;
	SetR16(rd, attr16);
}

void CPU::LD_HL_R8() {
	uint8_t rs = opcode & 0b111;
	uint8_t value = GetR8(rs);
	Write8(HL, value);
}

void CPU::LD_HL_N8() {
	Write8(HL, attr8);
}

void CPU::LD_R8_HL() {
	uint8_t rd = (opcode >> 3) & 0b111;
	uint8_t value = Read8(HL);
	SetR8(rd, value);
}

void CPU::LD_R16_A() {
	uint8_t rs = (opcode >> 4) & 0b11;
	uint16_t address = GetR16(rs);
	Write8(address, A);
}

void CPU::LD_N16_A() {
	Write8(attr16, A);
}

void CPU::LDH_N8_A() {
	Write8(0xff00 | attr8, A);
}

void CPU::LDH_C_A() {
	Write8(0xff00 | C, A);
}

void CPU::LD_A_R16() {
	uint8_t rs = (opcode >> 4) & 0b11;
	uint16_t address = GetR16(rs);
	A = Read8(address);
}

void CPU::LD_A_N16() {
	A = Read8(attr16);
}

void CPU::LDH_A_N8() {
	A = Read8(0xff00 | attr8);
}

void CPU::LDH_A_C() {
	A = Read8(0xff00 | C);
}

void CPU::LD_HLI_A() {
	Write8(HL, A);
	HL++;
}

void CPU::LD_HLD_A() {
	Write8(HL, A);
	HL--;
}

void CPU::LD_A_HLI() {
	A = Read8(HL);
	HL++;
}

void CPU::LD_A_HLD() {
	A = Read8(HL);
	HL--;
}

// Jump and Subroutines
void CPU::CALL_N16() {
	Push16(PC);
	PC = attr16;
}

void CPU::CALL_CC_N16() {
	if (CheckCondition((opcode >> 3) & 0b11)) {
		Push16(PC);
		PC = attr16;
		stepClock += 12;
	}
}

void CPU::JP_HL() {
	PC = HL;
}

void CPU::JP_N16() {
	PC = attr16;
}

void CPU::JP_CC_N16() {
	if (CheckCondition((opcode >> 3) & 0b11)) {
		stepClock += 4;
		PC = attr16;
	}
}

void CPU::JR_E8() {
	PC += (int8_t)attr8;
}

void CPU::JR_CC_E8() {
	if (CheckCondition((opcode >> 3) & 0b11)) {
		stepClock += 4;
		PC += (int8_t)attr8;
	}
}

void CPU::RET_CC() {
	if (CheckCondition((opcode >> 3) & 0b11)) {
		PC = Pop16();
		stepClock += 12;
	}
}

void CPU::RET() {
	PC = Pop16();
}

void CPU::RETI() {
	PC = Pop16();
	IME = 1;
}

void CPU::RST_VEC() {
	uint16_t address = opcode & 0b111000;
	Push16(PC);
	PC = address;
}

// Stack Operations Instructions
void CPU::ADD_HL_SP() {
	uint32_t value = SP;
	uint32_t temp = HL;
	uint32_t result = temp + value;
	HL = result;
	fSub = 0;
	fHalfCarry = ((temp & 0xfff) + (value & 0xfff)) > 0xfff;
	fCarry = result > 0xffff;
}

void CPU::ADD_SP_E8() {
	uint16_t temp = SP;
	int16_t value = (int16_t)((int8_t)attr8);
	uint16_t result = temp + value;
	SP = result;
	fZero = 0;
	fSub = 0;
	fHalfCarry = ((temp ^ value ^ result) & 0x10) == 0x10;
	fCarry = ((temp ^ value ^ result) & 0x100) == 0x100;
}

void CPU::DEC_SP() {
	SP--;
}

void CPU::INC_SP() {
	SP++;
}

void CPU::LD_SP_N16() {
	SP = attr16;
}

void CPU::LD_N16_SP() {
	Write16(attr16, SP);
}

void CPU::LD_HL_SP_E8() {
	uint16_t temp = SP;
	int16_t value = (int16_t)((int8_t)attr8);
	uint16_t result = temp + value;
	HL = result;
	fZero = 0;
	fSub = 0;
	fHalfCarry = ((temp ^ value ^ result) & 0x10) == 0x10;
	fCarry = ((temp ^ value ^ result) & 0x100) == 0x100;
}

void CPU::LD_SP_HL() {
	SP = HL;
}

void CPU::POP_AF() {
	AF = Pop16() & 0xfff0;
}

void CPU::POP_R16() {
	uint8_t rd = (opcode >> 4) & 0b11;
	uint16_t value = Pop16();
	SetR16(rd, value);
}

void CPU::PUSH_AF() {
	Push16(AF & 0xfff0);
}

void CPU::PUSH_R16() {
	uint8_t rs = (opcode >> 4) & 0b11;
	uint16_t value = GetR16(rs);
	Push16(value);
}

// Miscellaneous Instructions
void CPU::CCF() {
	fSub = 0;
	fHalfCarry = 0;
	fCarry = !fCarry;
}

void CPU::CPL() {
	A = ~A;
	fSub = 1;
	fHalfCarry = 1;
}

void CPU::DAA() {
	if (!fSub) {
		if (fCarry || A > 0x99) { A += 0x60; fCarry = 1; }
		if (fHalfCarry || (A & 0x0f) > 0x09) { A += 0x6; }
	}
	else {
		if (fCarry) { A -= 0x60; }
		if (fHalfCarry) { A -= 0x6; }
	}
	fZero = (A == 0);
	fHalfCarry = 0;
}

void CPU::DI() {
	IME = 0;
}

void CPU::EI() {
	stepClock += Step();
	if (opcode != 0xf3) {
		IME = 1;
	}
}

void CPU::HALT() {
	if (IME) {
		isHalting = true;
	}
	else {
		if (bus.IE & bus.IF & 0x1f) {
			std::cout << "HALT can bug out\n";
			haltBug = true;
		}
		else {
			isHalting = true;
		}
	}
	stepClock += 4;
}

void CPU::NOP() {}

void CPU::SCF() {
	fSub = 0;
	fHalfCarry = 0;
	fCarry = 1;
}

void CPU::STOP() {
	std::cout << "STOP: not implemented\n";
	Write8(0xff04, 0);
}

CPU::Opcode::Opcode(const std::string& assembler, Instruction inst, uint8_t length, uint8_t clock, uint8_t mClock)
	: assembler(assembler), inst(inst), length(length), clock(clock), mClock(mClock) {}