#include <cassert>
#include <tfhe.h>

void zero(bitspan_t src) {
	for (auto bit : src)
		write(bit, 0);
}

bit_t equals(bitspan_t a, bitspan_t b) {
	assert(a.size() == b.size());
	bit_t ret = make_bit();
	_unsafe_constant(ret, true);
	bit_t tmp = make_bit();
	for (int i = 0; i < a.size(); i++) {
		_xnor(tmp, a[i], b[i]);
		_and(ret, ret, tmp);
	}
	return ret;
}

bit_t is_zero(bitspan_t src) {
	bit_t ret = make_bit();
	_not(ret, src[0]);
	for (int i = 1; i < src.size(); i++)
		_andyn(ret, ret, src[i]);
	return ret;
}

bit_t is_nonzero(bitspan_t src) {
	bit_t ret = make_bit();
	_copy(ret, src[0]);
	for (int i = 1; i < src.size(); i++)
		_or(ret, ret, src[i]);
	return ret;
}

bit_t is_negative(bitspan_t src) {
	return src.last();
}

// No bounds checking is done!
void _memcpy(bitspan_t dst, bitspan_t src, size_t size) {
	for (size_t i = 0; i < size; i++)
		_copy(dst[i], src[i]);
}

void _copy(bitspan_t dst, bitspan_t src) {
	assert(dst.size() == src.size());
	_memcpy(dst, src, dst.size());
}

void increment_if(bitspan_t out, bit_t cond, bitspan_t src) {
	const auto size = src.size();
	// The code was generated with the help of verilog-to-glovebox, hence the
	// variable names.
	bit_t _00_ = make_bit();
	bit_t _01_ = make_bit();
	_unsafe_constant(_00_, 0);
	_copy(_01_, cond);
	for (int i = 0; i < size - 3; i += 2) {
		_and(_00_, src[i], _01_);
		_xor(out[i], src[i], _01_);
		_and(_01_, src[i + 1], _00_);
		_xor(out[i + 1], src[i + 1], _00_);
	}
	_nand(_00_, src[size - 2], _01_);
	_xor(out[size - 2], src[size - 2], _01_);
	_xnor(out[size - 1], src[size - 1], _00_);
}

void increment_if(bitspan_t src, const bit_t cond) {
	increment_if(src, cond, src);
}

void decrement_if(bitspan_t out, bit_t cond, bitspan_t src) {
	const auto size = src.size();
	// The code was generated with the help of verilog-to-glovebox, hence the
	// variable names.
	bit_t _00_ = make_bit();
	bit_t _01_ = make_bit();
	_unsafe_constant(_00_, 0);
	_not(_01_, cond);
	for (int i = 0; i < size - 3; i += 2) {
		_or(_00_, src[i], _01_);
		_xnor(out[i], src[i], _01_);
		_or(_01_, src[i + 1], _00_);
		_xnor(out[i + 1], src[i + 1], _00_);
	}
	_or(_00_, src[size - 2], _01_);
	_xnor(out[size - 2], src[size - 2], _01_);
	_xnor(out[size - 1], src[size - 1], _00_);
}

void decrement_if(bitspan_t src, const bit_t cond) {
	decrement_if(src, cond, src);
}