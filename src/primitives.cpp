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

bit_t equals(bitspan_t a, bitspan_t b, TFHEServerParams_t p) {
	assert(a.size() == b.size());
	bit_t ret = make_bit(p);
	constant(ret, true, p);
	bit_t tmp = make_bit(p);
	for (int i = 0; i < a.size(); i++) {
		_xnor(tmp, a[i], b[i], p);
		_and(ret, ret, tmp, p);
	}
	return ret;
}

bit_t is_zero(bitspan_t src, TFHEServerParams_t p) {
	bit_t ret = make_bit(p);
	_not(ret, src[0], p);
	for (int i = 1; i < src.size(); i++)
		_andyn(ret, ret, src[i], p);
	return ret;
}

bit_t is_nonzero(bitspan_t src, TFHEServerParams_t p) {
	bit_t ret = make_bit(p);
	_copy(ret, src[0], p);
	for (int i = 1; i < src.size(); i++)
		_or(ret, ret, src[i], p);
	return ret;
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

void incr_if(bitspan_t out, const bit_t cond, const bitspan_t src,
             TFHEServerParams_t p) {
	const auto size = src.size();
	// The code was generated with the help of verilog-to-tfhe, hence the
	// variable names.
	bit_t _00_ = make_bit(p);
	bit_t _01_ = make_bit(p);
	constant(_00_, 0, p);
	_copy(_01_, cond, p);
	for (int i = 0; i < size - 3; i += 2) {
		_and(_00_, src[i], _01_, p);
		_xor(out[i], src[i], _01_, p);
		_and(_01_, src[i + 1], _00_, p);
		_xor(out[i + 1], src[i + 1], _00_, p);
	}
	_nand(_00_, src[size - 2], _01_, p);
	_xor(out[size - 2], src[size - 2], _01_, p);
	_xnor(out[size - 1], src[size - 1], _00_, p);
}

void decr_if(bitspan_t out, const bit_t cond, const bitspan_t src,
             TFHEServerParams_t p) {
	const auto size = src.size();
	// The code was generated with the help of verilog-to-tfhe, hence the
	// variable names.
	bit_t _00_ = make_bit(p);
	bit_t _01_ = make_bit(p);
	constant(_00_, 0, p);
	_not(_01_, cond, p);
	for (int i = 0; i < size - 3; i += 2) {
		_or(_00_, src[i], _01_, p);
		_xnor(out[i], src[i], _01_, p);
		_or(_01_, src[i + 1], _00_, p);
		_xnor(out[i + 1], src[i + 1], _00_, p);
	}
	_or(_00_, src[size - 2], _01_, p);
	_xnor(out[size - 2], src[size - 2], _01_, p);
	_xnor(out[size - 1], src[size - 1], _00_, p);
}