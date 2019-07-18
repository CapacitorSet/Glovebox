#ifndef GLOVEBOX_PRIMITIVES_H
#define GLOVEBOX_PRIMITIVES_H

#include <tfhe.h>

void zero(bitspan_t src);

bit_t is_zero(bitspan_t src);
bit_t is_nonzero(bitspan_t src);
bit_t is_negative(bitspan_t src);

bit_t equals(bitspan_t a, bitspan_t b);

// No bounds checking is done!
void _memcpy(bitspan_t dst, bitspan_t src, size_t size);
void _copy(bitspan_t dst, bitspan_t src);
template <uint16_t N> void _copy(fixed_bitspan_t<N> dst, fixed_bitspan_t<N> src) {
	for (uint16_t i = 0; i < N; i++)
		_copy(dst[i], src[i]);
}

void increment_if(bitspan_t out, bit_t cond, bitspan_t src);
void decrement_if(bitspan_t out, bit_t cond, bitspan_t src);
// out = src
void increment_if(bitspan_t src, bit_t cond);
void decrement_if(bitspan_t src, bit_t cond);

void add8(fixed_bitspan_t<8> out, bit_t overflow, fixed_bitspan_t<8> a, fixed_bitspan_t<8> b);
void add16(fixed_bitspan_t<16> out, bit_t overflow, fixed_bitspan_t<16> a, fixed_bitspan_t<16> b);
void mul8(fixed_bitspan_t<16> out, fixed_bitspan_t<8> a, fixed_bitspan_t<8> b);
void mul16(fixed_bitspan_t<32> out, fixed_bitspan_t<16> a, fixed_bitspan_t<16> b);
void div8(fixed_bitspan_t<8> out, fixed_bitspan_t<8> a, fixed_bitspan_t<8> b);
void div16(fixed_bitspan_t<16> out, fixed_bitspan_t<16> a, fixed_bitspan_t<16> b);

#endif // GLOVEBOX_PRIMITIVES_H