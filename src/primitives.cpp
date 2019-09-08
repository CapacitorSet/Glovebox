#include <bitvec.h>
#include <cassert>
#include <tfhe.h>

void zero(bitvec_t src) {
	for (int i = 0; i < src.size(); i++)
		src[i] = 0;
}

bit_t equals(bitvec_t a, bitvec_t b) {
	assert(a.size() == b.size());
	bit_t ret = make_bit();
	_xnor(ret, a[0], b[0]);
	bit_t tmp = make_bit();
	for (int i = 1; i < a.size(); i++) {
		_xnor(tmp, a[i], b[i]);
		ret &= tmp;
	}
	return ret;
}

bit_t is_zero(bitvec_t src) {
	bit_t ret = make_bit();
	_copy(ret, src[0]);
	for (int i = 1; i < src.size(); i++)
		ret |= src[i];
	_not(ret, ret);
	return ret;
}
bit_t is_nonzero(bitvec_t src) {
	bit_t ret = make_bit();
	_copy(ret, src[0]);
	for (int i = 1; i < src.size(); i++)
		ret |= src[i];
	return ret;
}

bit_t is_negative(bitvec_t src) {
	return src.last();
}
bit_t is_positive(bitvec_t src) {
	bit_t ret = is_zero(src);
	// Neither zero nor negative
	_nor(ret, ret, is_negative(src));
	return ret;
}

gb::bitvec<3> compare(bitvec_t a, bitvec_t b) {
	assert(a.size() == b.size());
	gb::bitvec<3> ret = make_bitvec<3>();
	bit_t is_lt(ret[0]); // lesser than
	bit_t is_equal(ret[1]);
	bit_t is_gt(ret[2]); // greater than

	_andny(is_gt, a.last(), b.last());
	_andyn(is_lt, a.last(), b.last());

	bit_t tmp = make_bit();
	for (int i = a.size() - 1; i-- > 0;) {
		_andny(tmp, a[i], b[i]);
		_andny(tmp, is_gt, tmp);
		is_lt |= tmp;

		_andyn(tmp, a[i], b[i]);
		_andny(tmp, is_lt, tmp);
		is_gt |= tmp;
	}
	_nor(is_equal, is_lt, is_gt);
	return ret;
}

void sign_extend(bitvec_t dst, size_t dst_size, bitvec_t src, size_t src_size) {
	bit_t sign = src.last();
	for (size_t i = 0; i < dst_size; i++)
		if (i < src_size)
			_copy(dst[i], src[i]);
		else
			_copy(dst[i], sign);
}

// No bounds checking is done!
void _memcpy(bitvec_t dst, bitvec_t src, size_t size) {
	for (size_t i = 0; i < size; i++)
		_copy(dst[i], src[i]);
}

void _copy(bitvec_t dst, bitvec_t src) {
	assert(dst.size() == src.size());
	_memcpy(dst, src, dst.size());
}

void memimport(bitvec_t dst, const void *src, size_t len) {
	for (size_t i = 0; i < len; i++) {
		for (int j = 0; j < 8; j++)
			dst[i * 8 + j] = (((char *)src)[i] >> j) & 1;
	}
}
#if !GB_SERVER
void memexport(void *dst, bitvec_t src, size_t len) {
	for (size_t i = 0; i < len; i++) {
		char out = 0;
		for (int j = 0; j < 8; j++)
			out |= decrypt(src[i * 8 + j]) << j;
		((char *)dst)[i] = out;
	}
}
#endif

void increment_if(bitvec_t out, bit_t cond, bitvec_t src) {
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

void increment_if(bitvec_t src, const bit_t cond) {
	increment_if(src, cond, src);
}

void decrement_if(bitvec_t out, bit_t cond, bitvec_t src) {
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

void decrement_if(bitvec_t src, const bit_t cond) {
	decrement_if(src, cond, src);
}