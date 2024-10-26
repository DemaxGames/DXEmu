#include "Memory.h"

namespace DXEmu {
	class Memory {
	private:
		uint8_t mem[8 * 1024];

	public:
		int Write(uint16_t address, uint8_t value) {
			mem[address] = value;
		}
		uint8_t Read(uint16_t address) {
			return mem[address];

		}
		uint16_t ReadWord(uint16_t address) {
			uint16_t value = mem[address];
			value <<= 8;
			value += mem[address + 1];
			return value;
		}
	};
}