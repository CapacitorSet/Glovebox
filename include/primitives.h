#ifndef GLOVEBOX_PRIMITIVES_H
#define GLOVEBOX_PRIMITIVES_H

#include <tfhe.h>

void zero(bitvec_t src);

bit_t is_zero(bitvec_t src);
bit_t is_nonzero(bitvec_t src);
bit_t is_negative(bitvec_t src);

bit_t equals(bitvec_t a, bitvec_t b);

// No bounds checking is done!
void _memcpy(bitvec_t dst, bitvec_t src, size_t size);
void _copy(bitvec_t dst, bitvec_t src);
template <uint16_t N> void _copy(fixed_bitvec_t<N> dst, fixed_bitvec_t<N> src) {
	for (uint16_t i = 0; i < N; i++)
		_copy(dst[i], src[i]);
}

void increment_if(bitvec_t out, bit_t cond, bitvec_t src);
void decrement_if(bitvec_t out, bit_t cond, bitvec_t src);
// out = src
void increment_if(bitvec_t src, bit_t cond);
void decrement_if(bitvec_t src, bit_t cond);

void add8(fixed_bitvec_t<8> out, bit_t overflow, fixed_bitvec_t<8> a, fixed_bitvec_t<8> b);
void add16(fixed_bitvec_t<16> out, bit_t overflow, fixed_bitvec_t<16> a, fixed_bitvec_t<16> b);
void mul8(fixed_bitvec_t<16> out, fixed_bitvec_t<8> a, fixed_bitvec_t<8> b);
void mul16(fixed_bitvec_t<32> out, fixed_bitvec_t<16> a, fixed_bitvec_t<16> b);
void div8(fixed_bitvec_t<8> out, fixed_bitvec_t<8> a, fixed_bitvec_t<8> b);
void div16(fixed_bitvec_t<16> out, fixed_bitvec_t<16> a, fixed_bitvec_t<16> b);

#endif // GLOVEBOX_PRIMITIVES_H