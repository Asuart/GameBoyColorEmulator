#pragma once
#include <cstdint>
#include <string>
#include "Bus.h"

class Bus;

class CPU {
public:
	union {
		uint16_t AF;
		struct {
			union {
				uint8_t F;
				struct {
					uint8_t fUnused : 4;
					uint8_t fCarry : 1;
					uint8_t fHalfCarry : 1;
					uint8_t fSub : 1;
					uint8_t fZero : 1;
				};
			};
			uint8_t A;
		};
	};
	union {
		uint16_t BC;
		struct {
			uint8_t C;
			uint8_t B;
		};
	};
	union {
		uint16_t DE;
		struct {
			uint8_t E;
			uint8_t D;
		};
	};
	union {
		uint16_t HL;
		struct {
			uint8_t L;
			uint8_t H;
		};
	};
	uint16_t SP;
	uint16_t PC;

	uint8_t stepClock;

	uint8_t opcode, secondaryOpcode;
	union {
		uint16_t attr16;
		struct {
			uint8_t attr8;
			uint8_t attr8High;
		};
	};

	uint8_t IME;
	uint8_t dummyRead;
	bool isHalting;
	bool haltBug;

	Bus& bus;

	uint64_t clock;
	bool logAssembler = false;

	CPU(Bus& bus);

	void Reset();
	uint32_t Step();
	void HandleInterruption(uint16_t handlerAddress);
	bool HandleInterruptions();

private:
	// Help Functions
	uint8_t GetR8(uint8_t index);
	void SetR8(uint8_t index, uint8_t value);
	uint16_t GetR16(uint8_t index);
	void SetR16(uint8_t index, uint16_t value);
	uint8_t Read8(uint16_t address);
	void Write8(uint16_t address, uint8_t value);
	uint16_t Read16(uint16_t address);
	void Write16(uint16_t address, uint16_t value);
	uint16_t Pop16();
	void Push16(uint16_t value);
	bool CheckCondition(uint8_t condition);
	void ExecuteThroughTable();
	void ExecuteInline();
	void ExecutePrefixThroughTable();
	void ExecutePrefixInline();

	// Custom Instruction
	void UNH();
	void PREFIX();

	// 8-Bit Arithmetic and Logic Instructions
	void ADC_A_R8();
	void ADC_A_HL();
	void ADC_A_N8();
	void ADD_A_R8();
	void ADD_A_HL();
	void ADD_A_N8();
	void AND_A_R8();
	void AND_A_HL();
	void AND_A_N8();
	void CP_A_R8();
	void CP_A_HL();
	void CP_A_N8();
	void DEC_R8();
	void DEC_HL();
	void INC_R8();
	void INC_HL();
	void OR_A_R8();
	void OR_A_HL();
	void OR_A_N8();
	void SBC_A_R8();
	void SBC_A_HL();
	void SBC_A_N8();
	void SUB_A_R8();
	void SUB_A_HL();
	void SUB_A_N8();
	void XOR_A_R8();
	void XOR_A_HL();
	void XOR_A_N8();

	// 16-Bit Arithmetic Instructions
	void ADD_HL_R16();
	void DEC_R16();
	void INC_R16();

	// Bit Operations Instructions
	void BIT_U3_R8();
	void BIT_U3_HL();
	void RES_U3_R8();
	void RES_U3_HL();
	void SET_U3_R8();
	void SET_U3_HL();
	void SWAP_R8();
	void SWAP_HL();

	// Bit Shift Instructions
	void RL_R8();
	void RL_HL();
	void RL_A();
	void RLC_R8();
	void RLC_HL();
	void RLC_A();
	void RR_R8();
	void RR_HL();
	void RR_A();
	void RRC_R8();
	void RRC_HL();
	void RRC_A();
	void SLA_R8();
	void SLA_HL();
	void SRA_R8();
	void SRA_HL();
	void SRL_R8();
	void SRL_HL();

	// Load Instructions
	void LD_R8_R8();
	void LD_R8_N8();
	void LD_R16_N16();
	void LD_HL_R8();
	void LD_HL_N8();
	void LD_R8_HL();
	void LD_R16_A();
	void LD_N16_A();
	void LDH_N8_A();
	void LDH_C_A();
	void LD_A_R16();
	void LD_A_N16();
	void LDH_A_N8();
	void LDH_A_C();
	void LD_HLI_A();
	void LD_HLD_A();
	void LD_A_HLI();
	void LD_A_HLD();

	// Jump and Subroutines
	void CALL_N16();
	void CALL_CC_N16();
	void JP_HL();
	void JP_N16();
	void JP_CC_N16();
	void JR_E8();
	void JR_CC_E8();
	void RET_CC();
	void RET();
	void RETI();
	void RST_VEC();

	// Stack Operations Instructions
	void ADD_HL_SP();
	void ADD_SP_E8();
	void DEC_SP();
	void INC_SP();
	void LD_SP_N16();
	void LD_N16_SP();
	void LD_HL_SP_E8();
	void LD_SP_HL();
	void POP_AF();
	void POP_R16();
	void PUSH_AF();
	void PUSH_R16();

	// Miscellaneous Instructions
	void CCF();
	void CPL();
	void DAA();
	void DI();
	void EI();
	void HALT();
	void NOP();
	void SCF();
	void STOP();

	typedef void (CPU::* Instruction)();

	struct Opcode {
		const std::string assembler;
		const Instruction inst;
		const uint8_t length;
		const uint8_t clock;
		const uint8_t mClock;

		Opcode(const std::string& assenbler = "unh", Instruction inst = &CPU::UNH, uint8_t length = 1, uint8_t clock = 1, uint8_t mClock = 4);
	};

	const std::array<const Opcode, 0x100> baseOpcodeTable = {
		Opcode("nop", &CPU::NOP, 1, 4, 4), // 0x00
		Opcode("ld BC, n16", &CPU::LD_R16_N16, 3, 12, 12), // 0x01
		Opcode("ld [BC], A", &CPU::LD_R16_A, 1, 8, 8), // 0x02
		Opcode("inc BC", &CPU::INC_R16, 1, 8, 8), // 0x03
		Opcode("inc B", &CPU::INC_R8, 1, 4, 4), // 0x04
		Opcode("dec B", &CPU::DEC_R8, 1, 4, 4), // 0x05
		Opcode("ld B, n8", &CPU::LD_R8_N8, 2, 8, 8), // 0x06
		Opcode("rlca", &CPU::RLC_A, 1, 4, 4), // 0x07
		Opcode("ld [a16], SP", &CPU::LD_N16_SP, 3, 20, 20), // 0x08
		Opcode("add HL, BC", &CPU::ADD_HL_R16, 1, 8, 8), // 0x09
		Opcode("ld A, [BC]", &CPU::LD_A_R16, 1, 8, 8), // 0x0a
		Opcode("dec BC", &CPU::DEC_R16, 1, 8, 8), // 0x0b
		Opcode("inc C", &CPU::INC_R8, 1, 4, 4), // 0x0c
		Opcode("dec C", &CPU::DEC_R8, 1, 4, 4), // 0x0d
		Opcode("ld C, n8", &CPU::LD_R8_N8, 2,  8, 8), // 0x0e
		Opcode("rrca", &CPU::RRC_A, 1, 4, 4), // 0x0f

		Opcode("stop n8",&CPU::STOP,2,0,0), // 0x10
		Opcode("ld DE, n16", &CPU::LD_R16_N16, 3, 12, 12), // 0x11
		Opcode("ld [DE], A", &CPU::LD_R16_A, 1,8 ,8), // 0x12
		Opcode("inc DE", &CPU::INC_R16,1,8,8), // 0x13
		Opcode("inc D", &CPU::INC_R8, 1, 4, 4), // 0x14
		Opcode("dec D", &CPU::DEC_R8, 1, 4, 4), // 0x15
		Opcode("ld D, n8", &CPU::LD_R8_N8, 2, 8, 8), // 0x16
		Opcode("rla", &CPU::RL_A, 1, 4, 4), // 0x17
		Opcode("jr e8", &CPU::JR_E8, 2, 12, 12), // 0x18
		Opcode("add HL, DE", &CPU::ADD_HL_R16, 1, 8, 8), // 0x19
		Opcode("ld A, [DE]", &CPU::LD_A_R16, 1, 8, 8), // 0x1a
		Opcode("dec DE", &CPU::DEC_R16,1, 8, 8), // 0x1b
		Opcode("inc E", &CPU::INC_R8, 1,4, 4), // 0x1c
		Opcode("dec E", &CPU::DEC_R8, 1, 4, 4), // 0x1d
		Opcode("ld E, n8", &CPU::LD_R8_N8, 2, 8, 8), // 0x1e
		Opcode("rra", &CPU::RR_A, 1, 4, 4), // 0x1f

		Opcode("jr NZ, e8", &CPU::JR_CC_E8, 2, 8, 8), // 0x20
		Opcode("ld HL, n16", &CPU::LD_R16_N16, 3, 12, 12), // 0x21
		Opcode("ld [HL+], A", &CPU::LD_HLI_A, 1, 8, 8), // 0x22
		Opcode("inc HL", &CPU::INC_R16, 1, 8, 8), // 0x23
		Opcode("inc H", &CPU::INC_R8, 1, 4, 4), // 0x24
		Opcode("dec H", &CPU::DEC_R8, 1, 4, 4), // 0x25
		Opcode("ld H, n8", &CPU::LD_R8_N8, 2, 8, 8), // 0x26
		Opcode("daa", &CPU::DAA, 1, 4, 4), // 0x27
		Opcode("jr Z, e8", &CPU::JR_CC_E8, 2, 8, 8), // 0x28
		Opcode("add HL, HL", &CPU::ADD_HL_R16, 1, 8, 8), // 0x29
		Opcode("ld A, [HL+]", &CPU::LD_A_HLI, 1, 8, 8), // 0x2a
		Opcode("dec HL", &CPU::DEC_R16, 1, 8, 8), // 0x2b
		Opcode("inc L", &CPU::INC_R8, 1, 4, 4), // 0x2c
		Opcode("dec L", &CPU::DEC_R8, 1, 4, 4), // 0x2d
		Opcode("ld L, n8", &CPU::LD_R8_N8, 2, 8, 8), // 0x2e
		Opcode("cpl", &CPU::CPL, 1, 4, 4), // 0x2f

		Opcode("jr NC, e8", &CPU::JR_CC_E8, 2, 8, 8), // 0x30
		Opcode("ld SP, n16", &CPU::LD_R16_N16, 3, 12, 12), // 0x31
		Opcode("ld [HL-], A", &CPU::LD_HLD_A, 1, 8, 8), // 0x32
		Opcode("inc SP", &CPU::INC_R16, 1, 8, 8), // 0x33
		Opcode("inc [HL]", &CPU::INC_HL, 1, 12, 12), // 0x34
		Opcode("dec [HL]", &CPU::DEC_HL, 1, 12, 12), // 0x35
		Opcode("ld [HL], n8", &CPU::LD_HL_N8, 2, 12, 12), // 0x36
		Opcode("scf", &CPU::SCF, 1, 4, 4), // 0x37
		Opcode("jr C, e8", &CPU::JR_CC_E8, 2, 8, 8), // 0x38
		Opcode("add HL, SP", &CPU::ADD_HL_R16, 1, 8, 8), // 0x39
		Opcode("ld A, [HL-]", &CPU::LD_A_HLD, 1, 8, 8), // 0x3a
		Opcode("dec SP", &CPU::DEC_R16, 1, 8, 8), // 0x3b
		Opcode("inc A", &CPU::INC_R8, 1, 4, 4), // 0x3c
		Opcode("dec A", &CPU::DEC_R8, 1, 4, 4), // 0x3d
		Opcode("ld A, n8", &CPU::LD_R8_N8, 2, 8, 8), // 0x3e
		Opcode("ccf", &CPU::CCF, 1, 4, 4), // 0x3f

		Opcode("ld B, B", &CPU::LD_R8_R8, 1, 4, 4), // 0x40
		Opcode("ld B, C", &CPU::LD_R8_R8, 1, 4, 4), // 0x41
		Opcode("ld B, D", &CPU::LD_R8_R8, 1, 4, 4), // 0x42
		Opcode("ld B, E", &CPU::LD_R8_R8, 1, 4, 4), // 0x43
		Opcode("ld B, H", &CPU::LD_R8_R8, 1, 4, 4), // 0x44
		Opcode("ld B, L", &CPU::LD_R8_R8, 1, 4, 4), // 0x45
		Opcode("ld B, [HL]", &CPU::LD_R8_HL, 1, 8, 8), // 0x46
		Opcode("ld B, A", &CPU::LD_R8_R8, 1, 4, 4), // 0x47
		Opcode("ld C, B", &CPU::LD_R8_R8, 1, 4, 4), // 0x48
		Opcode("ld C, C", &CPU::LD_R8_R8, 1, 4, 4), // 0x49
		Opcode("ld C, D", &CPU::LD_R8_R8, 1, 4, 4), // 0x4a
		Opcode("ld C, E", &CPU::LD_R8_R8, 1, 4, 4), // 0x4b
		Opcode("ld C, H", &CPU::LD_R8_R8, 1, 4, 4), // 0x4c
		Opcode("ld C, L", &CPU::LD_R8_R8, 1, 4, 4), // 0x4d
		Opcode("ld C, [HL]", &CPU::LD_R8_HL, 1, 8, 8), // 0x4e
		Opcode("ld C, A", &CPU::LD_R8_R8, 1, 4, 4), // 0x4f

		Opcode("ld D, B", &CPU::LD_R8_R8, 1, 4, 4), // 0x50
		Opcode("ld D, C", &CPU::LD_R8_R8, 1, 4, 4), // 0x51
		Opcode("ld D, D", &CPU::LD_R8_R8, 1, 4, 4), // 0x52
		Opcode("ld D, E", &CPU::LD_R8_R8, 1, 4, 4), // 0x53
		Opcode("ld D, H", &CPU::LD_R8_R8, 1, 4, 4), // 0x54
		Opcode("ld D, L", &CPU::LD_R8_R8, 1, 4, 4), // 0x55
		Opcode("ld D, [HL]", &CPU::LD_R8_HL, 1, 8, 8), // 0x56
		Opcode("ld D, A", &CPU::LD_R8_R8, 1, 4, 4), // 0x57
		Opcode("ld E, B", &CPU::LD_R8_R8, 1, 4, 4), // 0x58
		Opcode("ld E, C", &CPU::LD_R8_R8, 1, 4, 4), // 0x59
		Opcode("ld E, D", &CPU::LD_R8_R8, 1, 4, 4), // 0x5a
		Opcode("ld E, E", &CPU::LD_R8_R8, 1, 4, 4), // 0x5b
		Opcode("ld E, H", &CPU::LD_R8_R8, 1, 4, 4), // 0x5c
		Opcode("ld E, L", &CPU::LD_R8_R8, 1, 4, 4), // 0x5d
		Opcode("ld E, [HL]", &CPU::LD_R8_HL, 1, 8, 8), // 0x5e
		Opcode("ld E, A", &CPU::LD_R8_R8, 1, 4, 4), // 0x5f

		Opcode("ld H, B", &CPU::LD_R8_R8, 1, 4, 4), // 0x60
		Opcode("ld H, C", &CPU::LD_R8_R8, 1, 4, 4), // 0x61
		Opcode("ld H, D", &CPU::LD_R8_R8, 1, 4, 4), // 0x62
		Opcode("ld H, E", &CPU::LD_R8_R8, 1, 4, 4), // 0x63
		Opcode("ld H, H", &CPU::LD_R8_R8, 1, 4, 4), // 0x64
		Opcode("ld H, L", &CPU::LD_R8_R8, 1, 4, 4), // 0x65
		Opcode("ld H, [HL]", &CPU::LD_R8_HL, 1, 8, 8), // 0x66
		Opcode("ld H, A", &CPU::LD_R8_R8, 1, 4, 4), // 0x67
		Opcode("ld L, B", &CPU::LD_R8_R8, 1, 4, 4), // 0x68
		Opcode("ld L, C", &CPU::LD_R8_R8, 1, 4, 4), // 0x69
		Opcode("ld L, D", &CPU::LD_R8_R8, 1, 4, 4), // 0x6a
		Opcode("ld L, E", &CPU::LD_R8_R8, 1, 4, 4), // 0x6b
		Opcode("ld L, H", &CPU::LD_R8_R8, 1, 4, 4), // 0x6c
		Opcode("ld L, L", &CPU::LD_R8_R8, 1, 4, 4), // 0x6d
		Opcode("ld L, [HL]", &CPU::LD_R8_HL, 1, 8, 8), // 0x6e
		Opcode("ld L, A", &CPU::LD_R8_R8, 1, 4, 4), // 0x6f

		Opcode("ld [HL], B", &CPU::LD_HL_R8, 1, 8, 8), // 0x70
		Opcode("ld [HL], C", &CPU::LD_HL_R8, 1, 8, 8), // 0x71
		Opcode("ld [HL], D", &CPU::LD_HL_R8, 1, 8, 8), // 0x72
		Opcode("ld [HL], E", &CPU::LD_HL_R8, 1, 8, 8), // 0x73
		Opcode("ld [HL], H", &CPU::LD_HL_R8, 1, 8, 8), // 0x74
		Opcode("ld [HL], L", &CPU::LD_HL_R8, 1, 8, 8), // 0x75
		Opcode("halt", &CPU::HALT, 1, 0, 0), // 0x76
		Opcode("ld [HL], A", &CPU::LD_HL_R8, 1, 8, 8), // 0x77
		Opcode("ld A, B", &CPU::LD_R8_R8, 1, 4, 4), // 0x78
		Opcode("ld A, C", &CPU::LD_R8_R8, 1, 4, 4), // 0x79
		Opcode("ld A, D", &CPU::LD_R8_R8, 1, 4, 4), // 0x7a
		Opcode("ld A, E", &CPU::LD_R8_R8, 1, 4, 4), // 0x7b
		Opcode("ld A, H", &CPU::LD_R8_R8, 1, 4, 4), // 0x7c
		Opcode("ld A, L", &CPU::LD_R8_R8, 1, 4, 4), // 0x7d
		Opcode("ld A, [HL]", &CPU::LD_R8_HL, 1, 8, 8), // 0x7e
		Opcode("ld A, A", &CPU::LD_R8_R8, 1, 4, 4), // 0x7f

		Opcode("add A, B", &CPU::ADD_A_R8, 1, 4, 4), // 0x80
		Opcode("add A, C", &CPU::ADD_A_R8, 1, 4, 4), // 0x81
		Opcode("add A, D", &CPU::ADD_A_R8, 1, 4, 4), // 0x82
		Opcode("add A, E", &CPU::ADD_A_R8, 1, 4, 4), // 0x83
		Opcode("add A, H", &CPU::ADD_A_R8, 1, 4, 4), // 0x84
		Opcode("add A, L", &CPU::ADD_A_R8, 1, 4, 4), // 0x85
		Opcode("add A, [HL]", &CPU::ADD_A_HL, 1, 8, 8), // 0x86
		Opcode("add A, A", &CPU::ADD_A_R8, 1, 4, 4), // 0x87
		Opcode("adc A, B", &CPU::ADC_A_R8, 1, 4, 4), // 0x88
		Opcode("adc A, C", &CPU::ADC_A_R8, 1, 4, 4), // 0x89
		Opcode("adc A, D", &CPU::ADC_A_R8, 1, 4, 4), // 0x8a
		Opcode("adc A, E", &CPU::ADC_A_R8, 1, 4, 4), // 0x8b
		Opcode("adc A, H", &CPU::ADC_A_R8, 1, 4, 4), // 0x8c
		Opcode("adc A, L", &CPU::ADC_A_R8, 1, 4, 4), // 0x8d
		Opcode("adc A, [HL]", &CPU::ADC_A_HL, 1, 8, 8), // 0x8e
		Opcode("adc A, A", &CPU::ADC_A_R8, 1, 4, 4), // 0x8f

		Opcode("sub A, B", &CPU::SUB_A_R8, 1, 4, 4), // 0x90
		Opcode("sub A, C", &CPU::SUB_A_R8, 1, 4, 4), // 0x91
		Opcode("sub A, D", &CPU::SUB_A_R8, 1, 4, 4), // 0x92
		Opcode("sub A, E", &CPU::SUB_A_R8, 1, 4, 4), // 0x93
		Opcode("sub A, H", &CPU::SUB_A_R8, 1, 4, 4), // 0x94
		Opcode("sub A, L", &CPU::SUB_A_R8, 1, 4, 4), // 0x95
		Opcode("sub A, [HL]", &CPU::SUB_A_HL, 1, 8, 8), // 0x96
		Opcode("sub A, A", &CPU::SUB_A_R8, 1, 4, 4), // 0x97
		Opcode("sbc A, B", &CPU::SBC_A_R8, 1, 4, 4), // 0x98
		Opcode("sbc A, C", &CPU::SBC_A_R8, 1, 4, 4), // 0x99
		Opcode("sbc A, D", &CPU::SBC_A_R8, 1, 4, 4), // 0x9a
		Opcode("sbc A, E", &CPU::SBC_A_R8, 1, 4, 4), // 0x9b
		Opcode("sbc A, H", &CPU::SBC_A_R8, 1, 4, 4), // 0x9c
		Opcode("sbc A, L", &CPU::SBC_A_R8, 1, 4, 4), // 0x9d
		Opcode("sbc A, [HL]", &CPU::SBC_A_HL, 1, 8, 8), // 0x9e
		Opcode("sbc A, A", &CPU::SBC_A_R8, 1, 4, 4), // 0x9f

		Opcode("and A, B", &CPU::AND_A_R8, 1, 4, 4), // 0xa0
		Opcode("and A, C", &CPU::AND_A_R8, 1, 4, 4), // 0xa1
		Opcode("and A, D", &CPU::AND_A_R8, 1, 4, 4), // 0xa2
		Opcode("and A, E", &CPU::AND_A_R8, 1, 4, 4), // 0xa3
		Opcode("and A, H", &CPU::AND_A_R8, 1, 4, 4), // 0xa4
		Opcode("and A, L", &CPU::AND_A_R8, 1, 4, 4), // 0xa5
		Opcode("and A, [HL]", &CPU::AND_A_HL, 1, 8, 8), // 0xa6
		Opcode("and A, A", &CPU::AND_A_R8, 1, 4, 4), // 0xa7
		Opcode("xor A, B", &CPU::XOR_A_R8, 1, 4, 4), // 0xa8
		Opcode("xor A, C", &CPU::XOR_A_R8, 1, 4, 4), // 0xa9
		Opcode("xor A, D", &CPU::XOR_A_R8, 1, 4, 4), // 0xaa
		Opcode("xor A, E", &CPU::XOR_A_R8, 1, 4, 4), // 0xab
		Opcode("xor A, H", &CPU::XOR_A_R8, 1, 4, 4), // 0xac
		Opcode("xor A, L", &CPU::XOR_A_R8, 1, 4, 4), // 0xad
		Opcode("xor A, [HL]", &CPU::XOR_A_HL, 1, 8, 8), // 0xae
		Opcode("xor A, A", &CPU::XOR_A_R8, 1, 4, 4), // 0xaf

		Opcode("or A, B", &CPU::OR_A_R8, 1, 4, 4), // 0xb0
		Opcode("or A, C", &CPU::OR_A_R8, 1, 4, 4), // 0xb1
		Opcode("or A, D", &CPU::OR_A_R8, 1, 4, 4), // 0xb2
		Opcode("or A, E", &CPU::OR_A_R8, 1, 4, 4), // 0xb3
		Opcode("or A, H", &CPU::OR_A_R8, 1, 4, 4), // 0xb4
		Opcode("or A, L", &CPU::OR_A_R8, 1, 4, 4), // 0xb5
		Opcode("or A, [HL]", &CPU::OR_A_HL, 1, 8, 8), // 0xb6
		Opcode("or A, A", &CPU::OR_A_R8, 1, 4, 4), // 0xb7
		Opcode("cp A, B", &CPU::CP_A_R8, 1, 4, 4), // 0xb8
		Opcode("cp A, C", &CPU::CP_A_R8, 1, 4, 4), // 0xb9
		Opcode("cp A, D", &CPU::CP_A_R8, 1, 4, 4), // 0xba
		Opcode("cp A, E", &CPU::CP_A_R8, 1, 4, 4), // 0xbb
		Opcode("cp A, H", &CPU::CP_A_R8, 1, 4, 4), // 0xbc
		Opcode("cp A, L", &CPU::CP_A_R8, 1, 4, 4), // 0xbd
		Opcode("cp A, [HL]", &CPU::CP_A_HL, 1, 8, 8), // 0xbe
		Opcode("cp A, A", &CPU::CP_A_R8, 1, 4, 4), // 0xbf

		Opcode("ret NZ", &CPU::RET_CC, 1, 8, 8), // 0xc0
		Opcode("pop BC", &CPU::POP_R16, 1, 12, 12), // 0xc1
		Opcode("jp NZ, a16", &CPU::JP_CC_N16, 3, 12, 12), // 0xc2
		Opcode("jp a16", &CPU::JP_N16, 3, 16, 16), // 0xc3
		Opcode("call NZ, a16", &CPU::CALL_CC_N16, 3, 12, 12), // 0xc4
		Opcode("push BC", &CPU::PUSH_R16, 1, 16, 16), // 0xc5
		Opcode("add, A, n8", &CPU::ADD_A_N8, 2, 8, 8), // 0xc6
		Opcode("rst $00", &CPU::RST_VEC, 1, 16, 16), // 0xc7
		Opcode("ret Z", &CPU::RET_CC, 1, 8, 8), // 0xc8
		Opcode("ret", &CPU::RET, 1, 16, 16), // 0xc9
		Opcode("jp Z, a16", &CPU::JP_CC_N16, 3, 12, 12), // 0xca
		Opcode("prefix", &CPU::PREFIX, 1, 0, 0), // 0xcb
		Opcode("call Z, a16", &CPU::CALL_CC_N16, 3, 12, 12), // 0xcc
		Opcode("call a16", &CPU::CALL_N16, 3, 24, 24), // 0xcd
		Opcode("adc A, n8", &CPU::ADC_A_N8, 2, 8, 8), // 0xce
		Opcode("rst $08", &CPU::RST_VEC, 1, 16, 16), // 0xcf

		Opcode("ret NC", &CPU::RET_CC, 1, 8, 8), // 0xd0
		Opcode("pop DE", &CPU::POP_R16, 1, 12, 12), // 0xd1
		Opcode("jp NC, a16", &CPU::JP_CC_N16, 3, 12, 12), // 0xd2
		Opcode(), // 0xd3
		Opcode("call NC, a16", &CPU::CALL_CC_N16, 3, 12, 12), // 0xd4
		Opcode("push DE", &CPU::PUSH_R16, 1, 16, 16), // 0xd5
		Opcode("sub A, n8", &CPU::SUB_A_N8, 2, 8, 8), // 0xd6
		Opcode("rst $10", &CPU::RST_VEC, 1, 16, 16), // 0xd7
		Opcode("ret C", &CPU::RET_CC, 1, 8, 8), // 0xd8
		Opcode("reti", &CPU::RETI, 1, 16, 16), // 0xd9
		Opcode("jp C, a16", &CPU::JP_CC_N16, 3, 12, 12), // 0xda
		Opcode(), // 0xdb
		Opcode("call C, a16", &CPU::CALL_CC_N16, 3, 12, 12), // 0xdc
		Opcode(), // 0xdd
		Opcode("sbc A, n8", &CPU::SBC_A_N8, 2, 8, 8), // 0xde
		Opcode("rst $18", &CPU::RST_VEC, 1, 16, 16), // 0xdf

		Opcode("ldh [a8], A", &CPU::LDH_N8_A, 2, 12, 12), // 0xe0
		Opcode("pop HL", &CPU::POP_R16, 1, 12, 12), // 0xe1
		Opcode("ld [C], A", &CPU::LDH_C_A, 1, 8, 8), // 0xe2
		Opcode(), // 0xe3
		Opcode(), // 0xe4
		Opcode("push HL", &CPU::PUSH_R16, 1, 16, 16), // 0xe5
		Opcode("and A, n8", &CPU::AND_A_N8, 2, 8, 8), // 0xe6
		Opcode("rst $20", &CPU::RST_VEC, 1, 16, 16), // 0xe7
		Opcode("add SP, e8", &CPU::ADD_SP_E8, 2, 16, 16), // 0xe8
		Opcode("jp HL", &CPU::JP_HL, 1, 4, 4), // 0xe9
		Opcode("ld [a16], a", &CPU::LD_N16_A, 3, 16, 16), // 0xea
		Opcode(), // 0xeb
		Opcode(), // 0xec
		Opcode(), // 0xed
		Opcode("xor A, n8", &CPU::XOR_A_N8, 2 ,8 ,8), // 0xee
		Opcode("rst $28", &CPU::RST_VEC, 1, 16, 16), // 0xef

		Opcode("ldh A, [a8]", &CPU::LDH_A_N8, 2, 12, 12), // 0xf0
		Opcode("pop AF", &CPU::POP_AF, 1, 12, 12), // 0xf1
		Opcode("ld A, [C]", &CPU::LDH_A_C, 1, 8, 8), // 0xf2
		Opcode("di", &CPU::DI, 1, 4, 4), // 0xf3
		Opcode(), // 0xf4
		Opcode("push AF", &CPU::PUSH_AF, 1, 16, 16), // 0xf5
		Opcode("or A, n8", &CPU::OR_A_N8, 2, 8, 8), // 0xf6
		Opcode("rst $30", &CPU::RST_VEC, 1, 16, 16), // 0xf7
		Opcode("ld Hl, SP + e8", &CPU::LD_HL_SP_E8, 2, 12, 12), // 0xf8
		Opcode("ld SP, HL", &CPU::LD_SP_HL, 1, 8, 8), // 0xf9
		Opcode("ld A, [a16]", &CPU::LD_A_N16, 3, 16, 16), // 0xfa
		Opcode("ei", &CPU::EI, 1, 4, 4), // 0xfb
		Opcode(), // 0xfc
		Opcode(), // 0xfd
		Opcode("cp A, n8", &CPU::CP_A_N8, 2, 8, 8), // 0xfe
		Opcode("rst $38", &CPU::RST_VEC, 1, 16, 16), // 0xff
	};

	const std::array<const Opcode, 0x100> prefixOpcodeTable = {
		Opcode("rlc B", &CPU::RLC_R8, 2, 8, 8), // 0x00
		Opcode("rlc C", &CPU::RLC_R8, 2, 8, 8), // 0x01
		Opcode("rlc D", &CPU::RLC_R8, 2, 8, 8), // 0x02
		Opcode("rlc E", &CPU::RLC_R8, 2, 8, 8), // 0x03
		Opcode("rlc H", &CPU::RLC_R8, 2, 8, 8), // 0x04
		Opcode("rlc L", &CPU::RLC_R8, 2, 8, 8), // 0x05
		Opcode("rlc [HL]", &CPU::RLC_HL, 2, 16, 16), // 0x06
		Opcode("rlc A", &CPU::RLC_R8, 2, 8, 8), // 0x07
		Opcode("rrc B", &CPU::RRC_R8, 2, 8, 8), // 0x08
		Opcode("rrc C", &CPU::RRC_R8, 2, 8, 8), // 0x09
		Opcode("rrc D", &CPU::RRC_R8, 2, 8, 8), // 0x0a
		Opcode("rrc E", &CPU::RRC_R8, 2, 8, 8), // 0x0b
		Opcode("rrc H", &CPU::RRC_R8, 2, 8, 8), // 0x0c
		Opcode("rrc L", &CPU::RRC_R8, 2, 8, 8), // 0x0d
		Opcode("rrc [HL]", &CPU::RRC_HL, 2, 16, 16), // 0x0e
		Opcode("rrc A", &CPU::RRC_R8, 2, 8, 8), // 0x0f

		Opcode("rl B", &CPU::RL_R8, 2, 8, 8), // 0x10
		Opcode("rl C", &CPU::RL_R8, 2, 8, 8), // 0x11
		Opcode("rl D", &CPU::RL_R8, 2, 8, 8), // 0x12
		Opcode("rl E", &CPU::RL_R8, 2, 8, 8), // 0x13
		Opcode("rl H", &CPU::RL_R8, 2, 8, 8), // 0x14
		Opcode("rl L", &CPU::RL_R8, 2, 8, 8), // 0x15
		Opcode("rl [HL]", &CPU::RL_HL, 2, 16, 16), // 0x16
		Opcode("rl A", &CPU::RL_R8, 2, 8, 8), // 0x17
		Opcode("rr B", &CPU::RR_R8, 2, 8, 8), // 0x18
		Opcode("rr C", &CPU::RR_R8, 2, 8, 8), // 0x19
		Opcode("rr D", &CPU::RR_R8, 2, 8, 8), // 0x1a
		Opcode("rr E", &CPU::RR_R8, 2, 8, 8), // 0x1b
		Opcode("rr H", &CPU::RR_R8, 2, 8, 8), // 0x1c
		Opcode("rr L", &CPU::RR_R8, 2, 8, 8), // 0x1d
		Opcode("rr [HL]", &CPU::RR_HL, 2, 16, 16), // 0x1e
		Opcode("rr A", &CPU::RR_R8, 2, 8, 8), // 0x1f

		Opcode("sla B", &CPU::SLA_R8, 2, 8, 8), // 0x20
		Opcode("sla C", &CPU::SLA_R8, 2, 8, 8), // 0x21
		Opcode("sla D", &CPU::SLA_R8, 2, 8, 8), // 0x22
		Opcode("sla E", &CPU::SLA_R8, 2, 8, 8), // 0x23
		Opcode("sla H", &CPU::SLA_R8, 2, 8, 8), // 0x24
		Opcode("sla L", &CPU::SLA_R8, 2, 8, 8), // 0x25
		Opcode("sla [HL]", &CPU::SLA_HL, 2, 16, 16), // 0x26
		Opcode("sla A", &CPU::SLA_R8, 2, 8, 8), // 0x27
		Opcode("sra B", &CPU::SRA_R8, 2, 8, 8), // 0x28
		Opcode("sra C", &CPU::SRA_R8, 2, 8, 8), // 0x29
		Opcode("sra D", &CPU::SRA_R8, 2, 8, 8), // 0x2a
		Opcode("sra E", &CPU::SRA_R8, 2, 8, 8), // 0x2b
		Opcode("sra H", &CPU::SRA_R8, 2, 8, 8), // 0x2c
		Opcode("sra L", &CPU::SRA_R8, 2, 8, 8), // 0x2d
		Opcode("sra [HL]", &CPU::SRA_HL, 2, 16, 16), // 0x2e
		Opcode("sra A", &CPU::SRA_R8, 2, 8, 8), // 0x2f

		Opcode("swap B", &CPU::SWAP_R8, 2, 8, 8), // 0x30
		Opcode("swap C", &CPU::SWAP_R8, 2, 8, 8), // 0x31
		Opcode("swap D", &CPU::SWAP_R8, 2, 8, 8), // 0x32
		Opcode("swap E", &CPU::SWAP_R8, 2, 8, 8), // 0x33
		Opcode("swap H", &CPU::SWAP_R8, 2, 8, 8), // 0x34
		Opcode("swap L", &CPU::SWAP_R8, 2, 8, 8), // 0x35
		Opcode("swap [HL]", &CPU::SWAP_HL, 2, 16, 16), // 0x36
		Opcode("swap A", &CPU::SWAP_R8, 2, 8, 8), // 0x37
		Opcode("srl B", &CPU::SRL_R8, 2, 8, 8), // 0x38
		Opcode("srl C", &CPU::SRL_R8, 2, 8, 8), // 0x39
		Opcode("srl D", &CPU::SRL_R8, 2, 8, 8), // 0x3a
		Opcode("srl E", &CPU::SRL_R8, 2, 8, 8), // 0x3b
		Opcode("srl H", &CPU::SRL_R8, 2, 8, 8), // 0x3c
		Opcode("srl L", &CPU::SRL_R8, 2, 8, 8), // 0x3d
		Opcode("srl [HL]", &CPU::SRL_HL, 2, 16, 16), // 0x3e
		Opcode("srl A", &CPU::SRL_R8, 2, 8, 8), // 0x3f

		Opcode("bit 0, B", &CPU::BIT_U3_R8, 2, 8, 8), // 0x40
		Opcode("bit 0, C", &CPU::BIT_U3_R8, 2, 8, 8), // 0x41
		Opcode("bit 0, D", &CPU::BIT_U3_R8, 2, 8, 8), // 0x42
		Opcode("bit 0, E", &CPU::BIT_U3_R8, 2, 8, 8), // 0x43
		Opcode("bit 0, H", &CPU::BIT_U3_R8, 2, 8, 8), // 0x44
		Opcode("bit 0, L", &CPU::BIT_U3_R8, 2, 8, 8), // 0x45
		Opcode("bit 0, [HL]", &CPU::BIT_U3_HL, 2, 12, 12), // 0x46
		Opcode("bit 0, A", &CPU::BIT_U3_R8, 2, 8, 8), // 0x47
		Opcode("bit 1, B", &CPU::BIT_U3_R8, 2, 8, 8), // 0x48
		Opcode("bit 1, C", &CPU::BIT_U3_R8, 2, 8, 8), // 0x49
		Opcode("bit 1, D", &CPU::BIT_U3_R8, 2, 8, 8), // 0x4a
		Opcode("bit 1, E", &CPU::BIT_U3_R8, 2, 8, 8), // 0x4b
		Opcode("bit 1, H", &CPU::BIT_U3_R8, 2, 8, 8), // 0x4c
		Opcode("bit 1, L", &CPU::BIT_U3_R8, 2, 8, 8), // 0x4d
		Opcode("bit 1, [HL]", &CPU::BIT_U3_HL, 2, 12, 12), // 0x4e
		Opcode("bit 1, A", &CPU::BIT_U3_R8, 2, 8, 8), // 0x4f

		Opcode("bit 2, B", &CPU::BIT_U3_R8, 2, 8, 8), // 0x50
		Opcode("bit 2, C", &CPU::BIT_U3_R8, 2, 8, 8), // 0x51
		Opcode("bit 2, D", &CPU::BIT_U3_R8, 2, 8, 8), // 0x52
		Opcode("bit 2, E", &CPU::BIT_U3_R8, 2, 8, 8), // 0x53
		Opcode("bit 2, H", &CPU::BIT_U3_R8, 2, 8, 8), // 0x54
		Opcode("bit 2, L", &CPU::BIT_U3_R8, 2, 8, 8), // 0x55
		Opcode("bit 2, [HL]", &CPU::BIT_U3_HL, 2, 12, 12), // 0x56
		Opcode("bit 2, A", &CPU::BIT_U3_R8, 2, 8, 8), // 0x57
		Opcode("bit 3, B", &CPU::BIT_U3_R8, 2, 8, 8), // 0x58
		Opcode("bit 3, C", &CPU::BIT_U3_R8, 2, 8, 8), // 0x59
		Opcode("bit 3, D", &CPU::BIT_U3_R8, 2, 8, 8), // 0x5a
		Opcode("bit 3, E", &CPU::BIT_U3_R8, 2, 8, 8), // 0x5b
		Opcode("bit 3, H", &CPU::BIT_U3_R8, 2, 8, 8), // 0x5c
		Opcode("bit 3, L", &CPU::BIT_U3_R8, 2, 8, 8), // 0x5d
		Opcode("bit 3, [HL]", &CPU::BIT_U3_HL, 2, 12,12), // 0x5e
		Opcode("bit 3, A", &CPU::BIT_U3_R8, 2, 8, 8), // 0x5f

		Opcode("bit 4, B", &CPU::BIT_U3_R8, 2, 8, 8), // 0x60
		Opcode("bit 4, C", &CPU::BIT_U3_R8, 2, 8, 8), // 0x61
		Opcode("bit 4, D", &CPU::BIT_U3_R8, 2, 8, 8), // 0x62
		Opcode("bit 4, E", &CPU::BIT_U3_R8, 2, 8, 8), // 0x63
		Opcode("bit 4, H", &CPU::BIT_U3_R8, 2, 8, 8), // 0x64
		Opcode("bit 4, L", &CPU::BIT_U3_R8, 2, 8, 8), // 0x65
		Opcode("bit 4, [HL]", &CPU::BIT_U3_HL, 2, 12, 12), // 0x66
		Opcode("bit 4, A", &CPU::BIT_U3_R8, 2, 8, 8), // 0x67
		Opcode("bit 5, B", &CPU::BIT_U3_R8, 2, 8, 8), // 0x68
		Opcode("bit 5, C", &CPU::BIT_U3_R8, 2, 8, 8), // 0x69
		Opcode("bit 5, D", &CPU::BIT_U3_R8, 2, 8, 8), // 0x6a
		Opcode("bit 5, E", &CPU::BIT_U3_R8, 2, 8, 8), // 0x6b
		Opcode("bit 5, H", &CPU::BIT_U3_R8, 2, 8, 8), // 0x6c
		Opcode("bit 5, L", &CPU::BIT_U3_R8, 2, 8, 8), // 0x6d
		Opcode("bit 5, [HL]", &CPU::BIT_U3_HL, 2, 12, 12), // 0x6e
		Opcode("bit 5, A", &CPU::BIT_U3_R8, 2, 8, 8), // 0x6f

		Opcode("bit 6, B", &CPU::BIT_U3_R8, 2, 8, 8), // 0x70
		Opcode("bit 6, C", &CPU::BIT_U3_R8, 2, 8, 8), // 0x71
		Opcode("bit 6, D", &CPU::BIT_U3_R8, 2, 8, 8), // 0x72
		Opcode("bit 6, E", &CPU::BIT_U3_R8, 2, 8, 8), // 0x73
		Opcode("bit 6, H", &CPU::BIT_U3_R8, 2, 8, 8), // 0x74
		Opcode("bit 6, L", &CPU::BIT_U3_R8, 2, 8, 8), // 0x75
		Opcode("bit 6, [HL]", &CPU::BIT_U3_HL, 2, 12, 12), // 0x76
		Opcode("bit 6, A", &CPU::BIT_U3_R8, 2, 8, 8), // 0x77
		Opcode("bit 7, B", &CPU::BIT_U3_R8, 2, 8, 8), // 0x78
		Opcode("bit 7, C", &CPU::BIT_U3_R8, 2, 8, 8), // 0x79
		Opcode("bit 7, D", &CPU::BIT_U3_R8, 2, 8, 8), // 0x7a
		Opcode("bit 7, E", &CPU::BIT_U3_R8, 2, 8, 8), // 0x7b
		Opcode("bit 7, H", &CPU::BIT_U3_R8, 2, 8, 8), // 0x7c
		Opcode("bit 7, L", &CPU::BIT_U3_R8, 2, 8, 8), // 0x7d
		Opcode("bit 7, [HL]", &CPU::BIT_U3_HL, 2, 12, 12), // 0x7e
		Opcode("bit 7, A", &CPU::BIT_U3_R8, 2, 8, 8), // 0x7f

		Opcode("res 0, B", &CPU::RES_U3_R8, 2, 8, 8), // 0x80
		Opcode("res 0, C", &CPU::RES_U3_R8, 2, 8, 8), // 0x81
		Opcode("res 0, D", &CPU::RES_U3_R8, 2, 8, 8), // 0x82
		Opcode("res 0, E", &CPU::RES_U3_R8, 2, 8, 8), // 0x83
		Opcode("res 0, H", &CPU::RES_U3_R8, 2, 8, 8), // 0x84
		Opcode("res 0, L", &CPU::RES_U3_R8, 2, 8, 8), // 0x85
		Opcode("res 0, [HL]", &CPU::RES_U3_HL, 2, 16, 16), // 0x86
		Opcode("res 0, A", &CPU::RES_U3_R8, 2, 8, 8), // 0x87
		Opcode("res 1, B", &CPU::RES_U3_R8, 2, 8, 8), // 0x88
		Opcode("res 1, C", &CPU::RES_U3_R8, 2, 8, 8), // 0x89
		Opcode("res 1, D", &CPU::RES_U3_R8, 2, 8, 8), // 0x8a
		Opcode("res 1, E", &CPU::RES_U3_R8, 2, 8, 8), // 0x8b
		Opcode("res 1, H", &CPU::RES_U3_R8, 2, 8, 8), // 0x8c
		Opcode("res 1, L", &CPU::RES_U3_R8, 2, 8, 8), // 0x8d
		Opcode("res 1, [HL]", &CPU::RES_U3_HL, 2, 16, 16), // 0x8e
		Opcode("res 1, A", &CPU::RES_U3_R8, 2, 8, 8), // 0x8f

		Opcode("res 2, B", &CPU::RES_U3_R8, 2, 8, 8), // 0x90
		Opcode("res 2, C", &CPU::RES_U3_R8, 2, 8, 8), // 0x91
		Opcode("res 2, D", &CPU::RES_U3_R8, 2, 8, 8), // 0x92
		Opcode("res 2, E", &CPU::RES_U3_R8, 2, 8, 8), // 0x93
		Opcode("res 2, H", &CPU::RES_U3_R8, 2, 8, 8), // 0x94
		Opcode("res 2, L", &CPU::RES_U3_R8, 2, 8, 8), // 0x95
		Opcode("res 2, [HL]", &CPU::RES_U3_HL, 2, 16, 16), // 0x96
		Opcode("res 2, A", &CPU::RES_U3_R8, 2, 8, 8), // 0x97
		Opcode("res 3, B", &CPU::RES_U3_R8, 2, 8, 8), // 0x98
		Opcode("res 3, C", &CPU::RES_U3_R8, 2, 8, 8), // 0x99
		Opcode("res 3, D", &CPU::RES_U3_R8, 2, 8, 8), // 0x9a
		Opcode("res 3, E", &CPU::RES_U3_R8, 2, 8, 8), // 0x9b
		Opcode("res 3, H", &CPU::RES_U3_R8, 2, 8, 8), // 0x9c
		Opcode("res 3, L", &CPU::RES_U3_R8, 2, 8, 8), // 0x9d
		Opcode("res 3, [HL]", &CPU::RES_U3_HL, 2, 16, 16), // 0x9e
		Opcode("res 3, A", &CPU::RES_U3_R8, 2, 8, 8), // 0x9f

		Opcode("res 4, B", &CPU::RES_U3_R8, 2, 8, 8), // 0xa0
		Opcode("res 4, C", &CPU::RES_U3_R8, 2, 8, 8), // 0xa1
		Opcode("res 4, D", &CPU::RES_U3_R8, 2, 8, 8), // 0xa2
		Opcode("res 4, E", &CPU::RES_U3_R8, 2, 8, 8), // 0xa3
		Opcode("res 4, H", &CPU::RES_U3_R8, 2, 8, 8), // 0xa4
		Opcode("res 4, L", &CPU::RES_U3_R8, 2, 8, 8), // 0xa5
		Opcode("res 4, [HL]", &CPU::RES_U3_HL, 2, 16, 16), // 0xa6
		Opcode("res 4, A", &CPU::RES_U3_R8, 2, 8, 8), // 0xa7
		Opcode("res 5, B", &CPU::RES_U3_R8, 2, 8, 8), // 0xa8
		Opcode("res 5, C", &CPU::RES_U3_R8, 2, 8, 8), // 0xa9
		Opcode("res 5, D", &CPU::RES_U3_R8, 2, 8, 8), // 0xaa
		Opcode("res 5, E", &CPU::RES_U3_R8, 2, 8, 8), // 0xab
		Opcode("res 5, H", &CPU::RES_U3_R8, 2, 8, 8), // 0xac
		Opcode("res 5, L", &CPU::RES_U3_R8, 2, 8, 8), // 0xad
		Opcode("res 5, [HL]", &CPU::RES_U3_HL, 2, 16, 16), // 0xae
		Opcode("res 5, A", &CPU::RES_U3_R8, 2, 8, 8), // 0xaf

		Opcode("res 6, B", &CPU::RES_U3_R8, 2, 8, 8), // 0xb0
		Opcode("res 6, C", &CPU::RES_U3_R8, 2, 8, 8), // 0xb1
		Opcode("res 6, D", &CPU::RES_U3_R8, 2, 8, 8), // 0xb2
		Opcode("res 6, E", &CPU::RES_U3_R8, 2, 8, 8), // 0xb3
		Opcode("res 6, H", &CPU::RES_U3_R8, 2, 8, 8), // 0xb4
		Opcode("res 6, L", &CPU::RES_U3_R8, 2, 8, 8), // 0xb5
		Opcode("res 6, [HL]", &CPU::RES_U3_HL, 2, 16, 16), // 0xb6
		Opcode("res 6, A", &CPU::RES_U3_R8, 2, 8, 8), // 0xb7
		Opcode("res 7, B", &CPU::RES_U3_R8, 2, 8, 8), // 0xb8
		Opcode("res 7, C", &CPU::RES_U3_R8, 2, 8, 8), // 0xb9
		Opcode("res 7, D", &CPU::RES_U3_R8, 2, 8, 8), // 0xba
		Opcode("res 7, E", &CPU::RES_U3_R8, 2, 8, 8), // 0xbb
		Opcode("res 7, H", &CPU::RES_U3_R8, 2, 8, 8), // 0xbc
		Opcode("res 7, L", &CPU::RES_U3_R8, 2, 8, 8), // 0xbd
		Opcode("res 7, [HL]", &CPU::RES_U3_HL, 2, 16, 16), // 0xbe
		Opcode("res 7, A", &CPU::RES_U3_R8, 2, 8, 8), // 0xbf

		Opcode("set 0, B", &CPU::SET_U3_R8, 2, 8, 8), // 0xc0
		Opcode("set 0, C", &CPU::SET_U3_R8, 2, 8, 8), // 0xc1
		Opcode("set 0, D", &CPU::SET_U3_R8, 2, 8, 8), // 0xc2
		Opcode("set 0, E", &CPU::SET_U3_R8, 2, 8, 8), // 0xc3
		Opcode("set 0, H", &CPU::SET_U3_R8, 2, 8, 8), // 0xc4
		Opcode("set 0, L", &CPU::SET_U3_R8, 2, 8, 8), // 0xc5
		Opcode("set 0, [HL]", &CPU::SET_U3_HL, 2, 16, 16), // 0xc6
		Opcode("set 0, A", &CPU::SET_U3_R8, 2, 8, 8), // 0xc7
		Opcode("set 1, B", &CPU::SET_U3_R8, 2, 8, 8), // 0xc8
		Opcode("set 1, C", &CPU::SET_U3_R8, 2, 8, 8), // 0xc9
		Opcode("set 1, D", &CPU::SET_U3_R8, 2, 8, 8), // 0xca
		Opcode("set 1, E", &CPU::SET_U3_R8, 2, 8, 8), // 0xcb
		Opcode("set 1, H", &CPU::SET_U3_R8, 2, 8, 8), // 0xcc
		Opcode("set 1, L", &CPU::SET_U3_R8, 2, 8, 8), // 0xcd
		Opcode("set 1, [HL]", &CPU::SET_U3_HL, 2, 16, 16), // 0xce
		Opcode("set 1, A", &CPU::SET_U3_R8, 2, 8, 8), // 0xcf

		Opcode("set 2, B", &CPU::SET_U3_R8, 2, 8, 8), // 0xd0
		Opcode("set 2, C", &CPU::SET_U3_R8, 2, 8, 8), // 0xd1
		Opcode("set 2, D", &CPU::SET_U3_R8, 2, 8, 8), // 0xd2
		Opcode("set 2, E", &CPU::SET_U3_R8, 2, 8, 8), // 0xd3
		Opcode("set 2, H", &CPU::SET_U3_R8, 2, 8, 8), // 0xd4
		Opcode("set 2, L", &CPU::SET_U3_R8, 2, 8, 8), // 0xd5
		Opcode("set 2, [HL]", &CPU::SET_U3_HL, 2, 16, 16), // 0xd6
		Opcode("set 2, A", &CPU::SET_U3_R8, 2, 8, 8), // 0xd7
		Opcode("set 3, B", &CPU::SET_U3_R8, 2, 8, 8), // 0xd8
		Opcode("set 3, C", &CPU::SET_U3_R8, 2, 8, 8), // 0xd9
		Opcode("set 3, D", &CPU::SET_U3_R8, 2, 8, 8), // 0xda
		Opcode("set 3, E", &CPU::SET_U3_R8, 2, 8, 8), // 0xdb
		Opcode("set 3, H", &CPU::SET_U3_R8, 2, 8, 8), // 0xdc
		Opcode("set 3, L", &CPU::SET_U3_R8, 2, 8, 8), // 0xdd
		Opcode("set 3, [HL]", &CPU::SET_U3_HL, 2, 16, 16), // 0xde
		Opcode("set 3, A", &CPU::SET_U3_R8, 2, 8, 8), // 0xdf

		Opcode("set 4, B", &CPU::SET_U3_R8, 2, 8, 8), // 0xe0
		Opcode("set 4, C", &CPU::SET_U3_R8, 2, 8, 8), // 0xe1
		Opcode("set 4, D", &CPU::SET_U3_R8, 2, 8, 8), // 0xe2
		Opcode("set 4, E", &CPU::SET_U3_R8, 2, 8, 8), // 0xe3
		Opcode("set 4, H", &CPU::SET_U3_R8, 2, 8, 8), // 0xe4
		Opcode("set 4, L", &CPU::SET_U3_R8, 2, 8, 8), // 0xe5
		Opcode("set 4, [HL]", &CPU::SET_U3_HL, 2, 16, 16), // 0xe6
		Opcode("set 4, A", &CPU::SET_U3_R8, 2, 8, 8), // 0xe7
		Opcode("set 5, B", &CPU::SET_U3_R8, 2, 8, 8), // 0xe8
		Opcode("set 5, C", &CPU::SET_U3_R8, 2, 8, 8), // 0xe9
		Opcode("set 5, D", &CPU::SET_U3_R8, 2, 8, 8), // 0xea
		Opcode("set 5, E", &CPU::SET_U3_R8, 2, 8, 8), // 0xeb
		Opcode("set 5, H", &CPU::SET_U3_R8, 2, 8, 8), // 0xec
		Opcode("set 5, L", &CPU::SET_U3_R8, 2, 8, 8), // 0xed
		Opcode("set 5, [HL]", &CPU::SET_U3_HL, 2, 16, 16), // 0xee
		Opcode("set 5, A", &CPU::SET_U3_R8, 2, 8, 8), // 0xef

		Opcode("set 6, B", &CPU::SET_U3_R8, 2, 8, 8), // 0xf0
		Opcode("set 6, C", &CPU::SET_U3_R8, 2, 8, 8), // 0xf1
		Opcode("set 6, D", &CPU::SET_U3_R8, 2, 8, 8), // 0xf2
		Opcode("set 6, E", &CPU::SET_U3_R8, 2, 8, 8), // 0xf3
		Opcode("set 6, H", &CPU::SET_U3_R8, 2, 8, 8), // 0xf4
		Opcode("set 6, L", &CPU::SET_U3_R8, 2, 8, 8), // 0xf5
		Opcode("set 6, [HL]", &CPU::SET_U3_HL, 2, 16, 16), // 0xf6
		Opcode("set 6, A", &CPU::SET_U3_R8, 2, 8, 8), // 0xf7
		Opcode("set 7, B", &CPU::SET_U3_R8, 2, 8, 8), // 0xf8
		Opcode("set 7, C", &CPU::SET_U3_R8, 2, 8, 8), // 0xf9
		Opcode("set 7, D", &CPU::SET_U3_R8, 2, 8, 8), // 0xfa
		Opcode("set 7, E", &CPU::SET_U3_R8, 2, 8, 8), // 0xfb
		Opcode("set 7, H", &CPU::SET_U3_R8, 2, 8, 8), // 0xfc
		Opcode("set 7, L", &CPU::SET_U3_R8, 2, 8, 8), // 0xfd
		Opcode("set 7, [HL]", &CPU::SET_U3_HL, 2, 16, 16), // 0xfe
		Opcode("set 7, A", &CPU::SET_U3_R8, 2, 8, 8), // 0xff
	};

	const std::array<uint8_t, 0x100> opcodeTimingTable = {
		1,3,2,2,1,1,2,1,5,2,2,2,1,1,2,1,
		0,3,2,2,1,1,2,1,3,2,2,2,1,1,2,1,
		2,3,2,2,1,1,2,1,2,2,2,2,1,1,2,1,
		2,3,2,2,3,3,3,1,2,2,2,2,1,1,2,1,
		1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
		1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
		1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
		2,2,2,2,2,2,1,2,1,1,1,1,1,1,2,1,
		1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
		1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
		1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
		1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
		2,3,3,4,3,4,2,4,2,4,3,0,3,6,2,4,
		2,3,3,0,3,4,2,4,2,4,3,0,3,0,2,4,
		3,3,2,0,0,4,2,4,4,1,4,0,0,0,2,4,
		3,3,2,1,0,4,2,4,3,2,4,1,0,0,2,4
	};
	const std::array<uint8_t, 0x100> prefixOpcodeTimingTable = {
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
		2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
		2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
		2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
		2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
		2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2
	};

	friend class Bus;
};