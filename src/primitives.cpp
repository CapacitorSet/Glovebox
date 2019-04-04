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
	for (size_t i = 0; i < size; i++)
		_copy(dst[i], src[i], p);
}

void _copy(bitspan_t dst, bitspan_t src, TFHEServerParams_t p) {
	assert(dst.size() == src.size());
	_memcpy(dst, src, dst.size(), p);
}

void incr_if(bitspan_t out, const bit_t cond, const bitspan_t src, TFHEServerParams_t p) {
	const auto size = src.size();
	// The code was generated with the help of verilog-to-tfhe, hence the variable names.
	bit_t _00_ = make_bit();
	bit_t _01_ = make_bit();
	for (int i = 0; i < size - 3; i += 2) {
		_and(_00_, src[i], _01_, p);
		_xor(out[i], src[i], _01_, p);
		_and(_01_, src[i+1], _00_, p);
		_xor(out[i+1], src[i+1], _00_, p);
	}
	_nand(_00_, src[size-2], _01_, p);
	_xor(out[size-2], src[size-2], _01_, p);
	_xnor(out[size-1], src[size-1], _00_, p);
}
