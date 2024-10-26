#include "CPU.h"

#define SET_FLAG_Z(value) (F = ((value) << 7) | (F & 0x7F))
#define SET_FLAG_N(value) (F = ((value) << 6) | (F & 0xBF))
#define SET_FLAG_H(value) (F = ((value) << 5) | (F & 0xDF))
#define SET_FLAG_C(value) (F = ((value) << 4) | (F & 0xEF))

#define FLAG_C (F & 0b00010000)
#define FLAG_H (F & 0b00100000)
#define FLAG_N (F & 0b01000000)
#define FLAG_Z (F & 0b10000000)

#define SYNC_WITH_CPU(clockDelta)

#define CP_N(value){SET_FLAG_Z((A - value) == 0x00);		\
	SET_FLAG_N(1);	\
	SET_FLAG_H((A & 0b00001000) >= (value & 0b00001000));		\
	SET_FLAG_C(A < value);				\
	SYNC_WITH_CPU(4)}													\

#define INC_N(value){SET_FLAG_Z(value + 1 == 0x00);\
	SET_FLAG_N(0);	\
	SET_FLAG_H((value & 0b00001000) & (value + 1 & 0b00001000));	\
	value++;\
	SYNC_WITH_CPU(4)} \

#define DEC_N(value){SET_FLAG_Z(value - 1 == 0x00);\
	SET_FLAG_N(1);	\
	SET_FLAG_H((value & 0b00001000) >= (value - 1 & 0b00001000));	\
	value--;\
	SYNC_WITH_CPU(4)} \

using namespace DXEmu;
namespace DXEmu {
	static union GPR{ //General Purpose Register
		struct {
			uint8_t L;
			uint8_t H;
		}minor;
		uint16_t major;
	};

	class CPU {
	private:
		uint8_t A, F;
		GPR BC, DE, HL;
		uint16_t PC = 0, SP;

		Memory mem;
	public:
		uint8_t MemoryRead(uint16_t address) {
			SYNC_WITH_CPU(4);
			return MemoryRead(address);
		}
		uint16_t MemoryReadWord(uint16_t address){
			uint16_t value = MemoryRead(address);
			value |= MemoryRead(address + 1) << 8;
		}
		void MemoryWrite(uint16_t address, uint8_t value) {
			MemoryWrite(address, value);
			SYNC_WITH_CPU(4);
		}
		void MemoryWriteWord(uint16_t address, uint8_t value) {
			MemoryWrite(address, value & 0xFF);
			MemoryWrite(address + 1, (value & 0xFF00) >> 8);
		}
		void Push(uint8_t value) {
			MemoryWrite(SP, value);
			SP--;
		}
		void PushWord(uint16_t value) {
			Push((value >> 8) & 0xFF);
			Push((value) & 0xFF);
			SP++;

			SYNC_WITH_CPU(4);
		}
		uint8_t Pop() {
			SP++;
			return MemoryRead(SP);
		}
		uint16_t PopWord() {
			SP -= 2;
			return MemoryReadWord(SP);
		}
		int Clock() {
			uint8_t opcode = MemoryRead(PC);
			PC++;
			switch (opcode) {
				case 0x06:						//LD nn, n
					BC.minor.H = MemoryRead(PC);
					break;
				case 0x0E:
					BC.minor.L = MemoryRead(PC);
					break;
				case 0x16:
					DE.minor.H = MemoryRead(PC);
					break;
				case 0x1E:
					DE.minor.L = MemoryRead(PC);
					break;
				case 0x26:
					HL.minor.H = MemoryRead(PC);
					break;
				case 0x2E:
					HL.minor.L = MemoryRead(PC);
					break;


				case 0x7F:						//LD r1,r2
					A = A;
					break;
				case 0x78:
					A = BC.minor.H;
					break;
				case 0x79:
					A = BC.minor.L;
					break;
				case 0x7A:
					A = DE.minor.H;
					break;
				case 0x7B:
					A = DE.minor.L;
					break;
				case 0x7C:
					A = HL.minor.H;
					break;
				case 0x7D:
					A = HL.minor.L;
					break;
				case 0x0A:
					A = BC.major;
					break;
				case 0x1A:
					A = DE.major;
					break;
				case 0x7E:
					A = HL.major;
					break;
				case 0xFA:
					A = MemoryReadWord(PC);
					break;
				case 0x3E:
					A = MemoryRead(PC);
					PC++;
					break;
				case 0x40:
					BC.minor.H = BC.minor.H;
					break;
				case 0x41:
					BC.minor.H = BC.minor.L;
					break;
				case 0x42:
					BC.minor.H = DE.minor.H;
					break;
				case 0x43:
					BC.minor.H = DE.minor.L;
					break;
				case 0x44:
					BC.minor.H = HL.minor.H;
					break;
				case 0x45:
					BC.minor.H = HL.minor.L;
					break;
				case 0x46:
					BC.minor.H = HL.major;
					break;
				case 0x48:
					BC.minor.L = BC.minor.H;
					break;
				case 0x49:
					BC.minor.L = BC.minor.L;
					break;
				case 0x4A:
					BC.minor.L = DE.minor.H;
					break;
				case 0x4B:
					BC.minor.L = DE.minor.L;
					break;
				case 0x4C:
					BC.minor.L = HL.minor.H;
					break;
				case 0x4D:
					BC.minor.L = HL.minor.L;
					break;
				case 0x4E:
					BC.minor.L = HL.major;
					break;
				case 0x50:
					DE.minor.H = BC.minor.H;
					break;
				case 0x51:
					DE.minor.H = BC.minor.L;
					break;
				case 0x52:
					DE.minor.H = DE.minor.H;
					break;
				case 0x53:
					DE.minor.H = DE.minor.L;
					break;
				case 0x54:
					DE.minor.H = HL.minor.H;
					break;
				case 0x55:
					DE.minor.H = HL.minor.L;
					break;
				case 0x56:
					DE.minor.H = HL.major;
					break;
				case 0x58:
					DE.minor.L = BC.minor.H;
					break;
				case 0x59:
					DE.minor.L = BC.minor.L;
					break;
				case 0x5A:
					DE.minor.L = DE.minor.H;
					break;
				case 0x5B:
					DE.minor.L = DE.minor.L;
					break;
				case 0x5C:
					DE.minor.L = HL.minor.H;
					break;
				case 0x5D:
					DE.minor.L = HL.minor.L;
					break;
				case 0x5E:
					DE.minor.L = HL.major;
					break;
				case 0x60:
					HL.minor.H = BC.minor.H;
					break;
				case 0x61:
					HL.minor.H = BC.minor.L;
					break;
				case 0x62:
					HL.minor.H = DE.minor.H;
					break;
				case 0x63:
					HL.minor.H = DE.minor.L;
					break;
				case 0x64:
					HL.minor.H = HL.minor.H;
					break;
				case 0x65:
					HL.minor.H = HL.minor.L;
					break;
				case 0x66:
					HL.minor.H = HL.major;
					break;
				case 0x68:
					HL.minor.L = BC.minor.H;
					break;
				case 0x69:
					HL.minor.L = BC.minor.L;
					break;
				case 0x6A:
					HL.minor.L = DE.minor.H;
					break;
				case 0x6B:
					HL.minor.L = DE.minor.L;
					break;
				case 0x6C:
					HL.minor.L = HL.minor.H;
					break;
				case 0x6D:
					HL.minor.L = HL.minor.L;
					break;
				case 0x6E:
					HL.minor.L = HL.major;
					break;
				case 0x70:
					HL.major = BC.minor.H;
					break;
				case 0x71:
					HL.major = BC.minor.L;
					break;
				case 0x72:
					HL.major = DE.minor.H;
					break;
				case 0x73:
					HL.major = DE.minor.L;
					break;
				case 0x74:
					HL.major = HL.minor.H;
					break;
				case 0x75:
					HL.major = HL.minor.L;
					break;
				case 0x36:
					HL.major = MemoryRead(PC);
					break;

				case 0x47:						//LD n,A
					BC.minor.H = A;
					break;
				case 0x4F:
					BC.minor.L = A;
					break;
				case 0x57:
					DE.minor.H = A;
					break;
				case 0x5F:
					DE.minor.L = A;
					break;
				case 0x67:
					HL.minor.H = A;
					break;
				case 0x6F:
					HL.minor.L = A;
					break;
				case 0x02:
					BC.major = A;
					break;
				case 0x12:
					DE.major = A;
					break;
				case 0x77:
					HL.major = A;
					break;
				case 0xEA:
					MemoryWrite(MemoryReadWord(PC), A);
					PC += 2;
					break;

				case 0xF2:					//LD A,(C)
					A = MemoryRead(0xFF00 + BC.minor.L);
					break;

				case 0xE2:					//LD (C),A
					MemoryWrite(0xFF00 + BC.minor.L, A);
					break;

				case 0x3A:					//LDD A,(HL)
					A = MemoryRead(HL.major);
					HL.major--;
					break;

				case 0x2A:					//LDI A,(HL)
					A = MemoryRead(HL.major);
					HL.major++;
					break;

				case 0x22:					//LDD A,(HL)
					MemoryWrite(HL.major, A);
					HL.major++;
					break;

				case 0xE0:					//LDH (n),A
					MemoryWrite(0xFF00 + MemoryRead(PC), A);
					PC++;
					break;

				case 0xF0:					//LDH A,(n)
					A = MemoryRead(0xFF00 + MemoryRead(PC));
					PC++;
					break;

				case 0x01:					//LD n,nn
					BC.major = MemoryReadWord(PC);
					PC += 2;
					break;
				case 0x11:
					DE.major = MemoryReadWord(PC);
					PC += 2;
					break;
				case 0x21:
					HL.major = MemoryReadWord(PC);
					PC += 2;
					break;
				case 0x31:
					SP = MemoryReadWord(PC);
					PC += 2;
					break;

				case 0xF9:					//LD SP,HL
					SP = HL.major;
					PC += 2;
					break;

				case 0xF8:					//LDHL SP,n
					uint8_t value = (signed char)MemoryRead(PC);
					value += SP;
					SET_FLAG_Z(0);
					SET_FLAG_N(0);
					SET_FLAG_H((value & 0xF) < (SP & 0xF));
					SET_FLAG_C((value & 0xFF) < (SP & 0xFF));

					HL.major = value;
					PC++;
					SYNC_WITH_CPU(4);
					break;

				case 0x08:					//LD (nn),SP
					MemoryWrite(MemoryReadWord(PC), SP);
					PC += 2;
					break;

				case 0xF5:					//PUSH nn
					Push(A);
					Push(F);
					SYNC_WITH_CPU(4);
					break;
				case 0xC5:
					PushWord(BC.major);
					break;
				case 0xD5:
					PushWord(DE.major);
					break;
				case 0xE5:
					PushWord(HL.major);
					break;

				case 0xF1:					//POP nn
					A = Pop();
					F = Pop();
					F &= 0xF0;
					break;
				case 0xC1:
					BC.major = PopWord();
					break;
				case 0xD1:
					DE.major = PopWord();
					break;
				case 0xE1:
					HL.major = PopWord();
					break;

				case 0x87:					//ADD A,n
					SET_FLAG_Z((A + A) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (A & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (A & 0b10000000));
					A += A;
					break;
				case 0x80:
					SET_FLAG_Z((A + BC.minor.H) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (BC.minor.H & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (BC.minor.H & 0b10000000));
					A += BC.minor.H;
					break;
				case 0x81:
					SET_FLAG_Z((A + BC.minor.L) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (BC.minor.L & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (BC.minor.L & 0b10000000));
					A += BC.minor.L;
					break;
				case 0x82:
					SET_FLAG_Z((A + DE.minor.H) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (DE.minor.H & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (DE.minor.H & 0b10000000));
					A += DE.minor.H;
					break;
				case 0x83:
					SET_FLAG_Z((A + DE.minor.L) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (DE.minor.L & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (DE.minor.L & 0b10000000));
					A += DE.minor.L;
					break;
				case 0x84:
					SET_FLAG_Z((A + HL.minor.H) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (HL.minor.H & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (HL.minor.H & 0b10000000));
					A += HL.minor.H;
					break;
				case 0x85:
					SET_FLAG_Z((A + HL.minor.L) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (HL.minor.L & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (HL.minor.L & 0b10000000));
					A += HL.minor.L;
					break;
				case 0x86:
					SET_FLAG_Z((A + HL.major) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (HL.major & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (HL.major & 0b10000000));
					A += HL.major;
					break;
				case 0xC6:
					uint8_t value = MemoryRead(PC);
					SET_FLAG_Z((A + value) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (value & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (value & 0b10000000));
					A += value;
					PC++;
					break;

				case 0x8F:					//ADC A,n
					SET_FLAG_Z((A + A + FLAG_C) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (A + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (A + FLAG_C & 0b10000000));
					A += A + FLAG_C;
					break;
				case 0x88:
					SET_FLAG_Z((A + BC.minor.H + FLAG_C) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (BC.minor.H + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (BC.minor.H + FLAG_C & 0b10000000));
					A += BC.minor.H + FLAG_C;
					break;
				case 0x89:
					SET_FLAG_Z((A + BC.minor.L + FLAG_C) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (BC.minor.L + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (BC.minor.L + FLAG_C & 0b10000000));
					A += BC.minor.L + FLAG_C;
					break;
				case 0x8A:
					SET_FLAG_Z((A + DE.minor.H + FLAG_C) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (DE.minor.H + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (DE.minor.H + FLAG_C & 0b10000000));
					A += DE.minor.H + FLAG_C;
					break;
				case 0x8B:
					SET_FLAG_Z((A + DE.minor.L + FLAG_C) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (DE.minor.L + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (DE.minor.L + FLAG_C & 0b10000000));
					A += DE.minor.L + FLAG_C;
					break;
				case 0x8C:
					SET_FLAG_Z((A + HL.minor.H + FLAG_C) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (HL.minor.H + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (HL.minor.H + FLAG_C & 0b10000000));
					A += HL.minor.H + FLAG_C;
					break;
				case 0x8D:
					SET_FLAG_Z((A + HL.minor.L + FLAG_C) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (HL.minor.L + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (HL.minor.L + FLAG_C & 0b10000000));
					A += HL.minor.L + FLAG_C;
					break;
				case 0x8E:
					SET_FLAG_Z((A + HL.major + FLAG_C) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (HL.major + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (HL.major + FLAG_C & 0b10000000));
					A += HL.major + FLAG_C;
					break;
				case 0xCE:
					uint8_t value = MemoryRead(PC) + FLAG_C;
					SET_FLAG_Z((A + value) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) & (value & 0b00001000));
					SET_FLAG_C((A & 0b10000000) & (value & 0b10000000));
					A += value;
					PC++;
					break;

				case 0x97:					//ADD A,n
					SET_FLAG_Z((A - A) == 0x00);
					SET_FLAG_N(1);
					SET_FLAG_H((A & 0b00001000) >= (A & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (A & 0b10000000));
					A -= A;
					break;
				case 0x90:
					SET_FLAG_Z((A - BC.minor.H) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (BC.minor.H & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (BC.minor.H & 0b10000000));
					A -= BC.minor.H;
					break;
				case 0x91:
					SET_FLAG_Z((A - BC.minor.L) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (BC.minor.L & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (BC.minor.L & 0b10000000));
					A -= BC.minor.L;
					break;
				case 0x92:
					SET_FLAG_Z((A - DE.minor.H) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (DE.minor.H & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (DE.minor.H & 0b10000000));
					A -= DE.minor.H;
					break;
				case 0x93:
					SET_FLAG_Z((A - DE.minor.L) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (DE.minor.L & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (DE.minor.L & 0b10000000));
					A -= DE.minor.L;
					break;
				case 0x94:
					SET_FLAG_Z((A - HL.minor.H) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (HL.minor.H & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (HL.minor.H & 0b10000000));
					A -= HL.minor.H;
					break;
				case 0x95:
					SET_FLAG_Z((A - HL.minor.L) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (HL.minor.L & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (HL.minor.L & 0b10000000));
					A -= HL.minor.L;
					break;
				case 0x96:
					SET_FLAG_Z((A - HL.major) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (HL.major & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (HL.major & 0b10000000));
					A += HL.major;
					break;
				case 0xD6:
					uint8_t value = MemoryRead(PC);
					SET_FLAG_Z((A - value) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (value & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (value & 0b10000000));
					A -= value;
					PC++;
					break;

				case 0x9F:					//SBC A,n
					SET_FLAG_Z((A - (A + FLAG_C)) == 0x00);
					SET_FLAG_N(1);
					SET_FLAG_H((A & 0b00001000) >= (A + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (A + FLAG_C & 0b10000000));
					A -= A + FLAG_C;
					break;
				case 0x98:
					SET_FLAG_Z((A - (BC.minor.H + FLAG_C)) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (BC.minor.H + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (BC.minor.H + FLAG_C & 0b10000000));
					A -= BC.minor.H + FLAG_C;
					break;
				case 0x99:
					SET_FLAG_Z((A - (BC.minor.L + FLAG_C)) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (BC.minor.L + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (BC.minor.L + FLAG_C & 0b10000000));
					A -= BC.minor.L + FLAG_C;
					break;
				case 0x9A:
					SET_FLAG_Z((A - (DE.minor.H + FLAG_C)) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (DE.minor.H + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (DE.minor.H + FLAG_C & 0b10000000));
					A -= DE.minor.H + FLAG_C;
					break;
				case 0x9B:
					SET_FLAG_Z((A - (DE.minor.L + FLAG_C)) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (DE.minor.L + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (DE.minor.L + FLAG_C & 0b10000000));
					A -= DE.minor.L + FLAG_C;
					break;
				case 0x9C:
					SET_FLAG_Z((A - (HL.minor.H + FLAG_C)) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (HL.minor.H + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (HL.minor.H + FLAG_C & 0b10000000));
					A -= HL.minor.H + FLAG_C;
					break;
				case 0x9D:
					SET_FLAG_Z((A - (HL.minor.L + FLAG_C)) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (HL.minor.L + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (HL.minor.L + FLAG_C & 0b10000000));
					A -= HL.minor.L + FLAG_C;
					break;
				case 0x9E:
					SET_FLAG_Z((A - (HL.major + FLAG_C)) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (HL.major + FLAG_C & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (HL.major + FLAG_C & 0b10000000));
					A += HL.major + FLAG_C;
					break;
				case 0xDE:
					uint8_t value = MemoryRead(PC) + FLAG_C;
					SET_FLAG_Z((A - value) == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H((A & 0b00001000) >= (value & 0b00001000));
					SET_FLAG_C((A & 0b10000000) >= (value & 0b10000000));
					A -= value;
					PC++;
					break;

				case 0xA7:						//AND n
					A &= A;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(1);
					SET_FLAG_C(0);
					break;
				case 0xA0:
					A &= BC.minor.H;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(1);
					SET_FLAG_C(0);
					break;
				case 0xA1:
					A &= BC.minor.L;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(1);
					SET_FLAG_C(0);
					break;
				case 0xA2:
					A &= DE.minor.H;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(1);
					SET_FLAG_C(0);
					break;
				case 0xA3:
					A &= DE.minor.L;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(1);
					SET_FLAG_C(0);
					break;
				case 0xA4:
					A &= HL.minor.H;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(1);
					SET_FLAG_C(0);
					break;
				case 0xA5:
					A &= HL.minor.L;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(1);
					SET_FLAG_C(0);
					break;
				case 0xA6:
					A &= HL.major;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(1);
					SET_FLAG_C(0);
					break;
				case 0xE6:
					A &= mem.Read(PC);
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(1);
					SET_FLAG_C(0);
					break;

				case 0xB7:						//OR n
					A |= A;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xB0:
					A |= BC.minor.H;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xB1:
					A |= BC.minor.L;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xB2:
					A |= DE.minor.H;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xB3:
					A |= DE.minor.L;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xB4:
					A |= HL.minor.H;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xB5:
					A |= HL.minor.L;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xB6:
					A |= HL.major;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xF6:
					A |= MemoryRead(PC);
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;

				case 0xAF:						//XOR n
					A ^= A;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xA8:
					A ^= BC.minor.H;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xA9:
					A ^= BC.minor.L;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xAA:
					A ^= DE.minor.H;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xAB:
					A ^= DE.minor.L;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xAC:
					A ^= HL.minor.H;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xAD:
					A ^= HL.minor.L;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xAE:
					A ^= HL.major;
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;
				case 0xEE:
					A ^= mem.Read(PC);
					SET_FLAG_Z(A == 0x00);
					SET_FLAG_N(0);
					SET_FLAG_H(0);
					SET_FLAG_C(0);
					break;

				case 0xBF:						//CP n
					CP_N(A);
					break;
				case 0xB8:
					CP_N(BC.minor.H);
					break;
				case 0xB9:
					CP_N(BC.minor.L);
					break;
				case 0xBA:
					CP_N(DE.minor.H);
					break;
				case 0xBB:
					CP_N(DE.minor.L);
					break;
				case 0xBC:
					CP_N(HL.minor.H);
					break;
				case 0xBD:
					CP_N(HL.minor.L);
					break;
				case 0xBE:
					CP_N(HL.major);
					break;
				case 0xFE:
					CP_N(MemoryRead(PC));
					break;

				case 0x3C:						//INC n
					INC_N(A);
					break;
				case 0x04:
					INC_N(BC.minor.H);
					break;
				case 0x0C:
					INC_N(BC.minor.L);
					break;
				case 0x14:
					INC_N(DE.minor.H);
					break;
				case 0x1C:
					INC_N(DE.minor.L);
					break;
				case 0x24:
					INC_N(HL.minor.H);
					break;
				case 0x2C:
					INC_N(HL.minor.L);
					break;
				case 0x34:
					uint8_t value = MemoryRead(HL.major);
					INC_N(value);
					MemoryWrite(HL.major, value);
					break;

				case 0x3D:						//DEC n
					DEC_N(A);
					break;
				case 0x05:
					DEC_N(BC.minor.H);
					break;
				case 0x0D:
					DEC_N(BC.minor.L);
					break;
				case 0x15:
					DEC_N(DE.minor.H);
					break;
				case 0x1D:
					DEC_N(DE.minor.L);
					break;
				case 0x25:
					DEC_N(HL.minor.H);
					break;
				case 0x2D:
					DEC_N(HL.minor.L);
					break;
				case 0x35:
					uint8_t value = MemoryRead(HL.major);
					DEC_N(value);
					MemoryWrite(HL.major, value);
					break;

				//WERE STARTIN 16-BIT ARITHMETIC YEAAAAAAAAAAHHH!!!!!!!!!!
			}
		}
	};
}