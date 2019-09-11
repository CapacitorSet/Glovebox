#ifndef GLOVEBOX_PRIMITIVES_H
#define GLOVEBOX_PRIMITIVES_H

#include <tfhe.h>

void zero(bitvec_t src);

bit_t is_zero(bitvec_t src);
bit_t is_nonzero(bitvec_t src);
bit_t is_negative(bitvec_t src);
bit_t is_positive(bitvec_t src);

bit_t equals(bitvec_t a, bitvec_t b);
// {is_lesser, is_equal, is_greater}
gb::bitvec<3> compare(bitvec_t a, bitvec_t b);

void sign_extend(bitvec_t dst, size_t dst_size, bitvec_t src, size_t src_size);
void abs(bitvec_t dst);

// No bounds checking is done!
void _memcpy(bitvec_t dst, bitvec_t src, size_t size);
void _copy(bitvec_t dst, bitvec_t src);
template <uint16_t N> void _copy(gb::bitvec<N> dst, gb::bitvec<N> src) {
	for (uint16_t i = 0; i < N; i++)
		_copy(dst[i], src[i]);
}

// Import `len` bytes into `dst`
void memimport(bitvec_t dst, const void *src, size_t len);
#if !GB_SERVER
void memexport(void *dst, bitvec_t src, size_t len);
#endif

void increment_if(bitvec_t out, bit_t cond, bitvec_t src);
void decrement_if(bitvec_t out, bit_t cond, bitvec_t src);
// out = src
void increment_if(bitvec_t src, bit_t cond);
void decrement_if(bitvec_t src, bit_t cond);

void add8(gb::bitvec<8> out, bit_t overflow, gb::bitvec<8> a, gb::bitvec<8> b);
void add16(gb::bitvec<16> out, bit_t overflow, gb::bitvec<16> a, gb::bitvec<16> b);
void mul8(gb::bitvec<16> out, gb::bitvec<8> a, gb::bitvec<8> b);
void mul16(gb::bitvec<32> out, gb::bitvec<16> a, gb::bitvec<16> b);
void div8(gb::bitvec<8> out, gb::bitvec<8> a, gb::bitvec<8> b);
void div16(gb::bitvec<16> out, gb::bitvec<16> a, gb::bitvec<16> b);

#endif // GLOVEBOX_PRIMITIVES_H