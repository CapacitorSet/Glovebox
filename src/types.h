#ifndef FHE_TOOLS_TYPES_H
#define FHE_TOOLS_TYPES_H

#include <cstdint>
#include "tfhe.h"

class Int {
public:
	static Int newU8(TFHEParams p) { return Int(8, false, p); }
	static Int newU8(uint8_t n, TFHEParams p) {
		auto ret = Int::newU8(p);
		ret.writeU8(n);
		return ret;
	}
	Int(uint8_t size, bool isSigned, TFHEParams p);

	void writeU8(uint8_t);

	void add(Int, Int);
	void copy(Int);

#if DEBUG
	void print();
#endif
private:
	uint8_t size;
	bool isSigned;
	bits_t data;
	TFHEParams p;
};

#endif //FHE_TOOLS_TYPES_H
