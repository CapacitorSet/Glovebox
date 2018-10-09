#include <cassert>
#include <unistd.h>
#include <cstring>
#include <istream>
#include <types.h>

ClientInt::ClientInt(uint8_t _size, bool _isSigned, TFHEClientParams_t _p) : p(_p) {
	size = _size;
	isSigned = _isSigned;
	data = make_bits(_size, p);
}

void ClientInt::writeU8(uint8_t val) {
	assert(size == 8);
	assert(!isSigned);
	for (int i = 0; i < 8; i++) {
		constant(&data[i], (val >> i) & 1, p);
	}
}

// todo: document that it doesn't export a header
void ClientInt::exportToFile(FILE *out) {
	for (int i = 0; i < size; i++)
		export_gate_bootstrapping_ciphertext_toFile(out, &data[i], p.params);
}

uint8_t ClientInt::toU8() {
	assert(size == 8);
	assert(!isSigned);
	uint8_t ret = 0;
	for (int i = 0; i < 8; i++) {
		ret |= (decrypt(&data[i], p) & 1) >> i;
	}
	return ret;
}