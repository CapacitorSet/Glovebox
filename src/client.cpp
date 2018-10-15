#include <cassert>
#include <cstring>
#include <istream>
#include <types.h>
#include <unistd.h>

void ClientInt::writeU8(uint8_t val) {
	assert(size == 8);
	assert(!isSigned);
	for (int i = 0; i < 8; i++) {
		constant(&data[i], (val >> i) & 1, p);
	}
}

uint8_t ClientInt::toU8() {
	assert(size == 8);
	assert(!isSigned);
	uint8_t ret = 0;
	for (int i = 0; i < 8; i++) {
		ret |= (decrypt(&data[i], p) & 1) << i;
	}
	return ret;
}