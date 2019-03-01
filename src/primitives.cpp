#include <cassert>
#include <tfhe.h>

void zero(bitspan_t src, TFHEClientParams_t p) {
	for (auto bit : src)
		encrypt(bit, 0, p);
}

void zero(bitspan_t src, TFHEServerParams_t p) {
	for (auto bit : src)
		constant(bit, 0, p);
}

// No bounds checking is done!
void _memcpy(bitspan_t dst, bitspan_t src, size_t size, TFHEServerParams_t p) {
	for (int i = 0; i < size; i++)
		_copy(dst[i], src[i], p);
}

void _copy(bitspan_t dst, bitspan_t src, TFHEServerParams_t p) {
	assert(dst.size() == src.size());
	_memcpy(dst, src, dst.size(), p);
}