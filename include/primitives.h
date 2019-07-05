#ifndef GLOVEBOX_PRIMITIVES_H
#define GLOVEBOX_PRIMITIVES_H

#include <tfhe.h>

void zero(bitspan_t src, ClientParams);
void zero(bitspan_t src, ServerParams = default_server_params);

bit_t is_zero(bitspan_t src, WeakParams = default_weak_params);
bit_t is_nonzero(bitspan_t src, WeakParams = default_weak_params);

bit_t equals(bitspan_t a, bitspan_t b, WeakParams = default_weak_params);

// No bounds checking is done!
void _memcpy(bitspan_t dst, bitspan_t src, size_t size,
             WeakParams = default_weak_params);
void _copy(bitspan_t dst, bitspan_t src, WeakParams = default_weak_params);
template <uint16_t N>
void _copy(fixed_bitspan_t<N> dst, fixed_bitspan_t<N> src,
           WeakParams p = default_weak_params) {
	for (uint16_t i = 0; i < N; i++)
		_copy(dst[i], src[i], p);
}

void incr_if(bitspan_t out, bit_t cond, bitspan_t src, WeakParams);
void decr_if(bitspan_t out, bit_t cond, bitspan_t src, WeakParams);

void add8(fixed_bitspan_t<8> out, bit_t overflow, fixed_bitspan_t<8> a,
          fixed_bitspan_t<8> b, WeakParams);
void add16(fixed_bitspan_t<16> out, bit_t overflow, fixed_bitspan_t<16> a,
           fixed_bitspan_t<16> b, WeakParams);
void mul8(fixed_bitspan_t<16> out, fixed_bitspan_t<8> a, fixed_bitspan_t<8> b,
          WeakParams);
void mul16(fixed_bitspan_t<32> out, fixed_bitspan_t<16> a,
           fixed_bitspan_t<16> b, WeakParams);
void div8(fixed_bitspan_t<8> out, fixed_bitspan_t<8> a, fixed_bitspan_t<8> b,
          WeakParams);
void div16(fixed_bitspan_t<16> out, fixed_bitspan_t<16> a,
           fixed_bitspan_t<16> b, WeakParams);

#endif // GLOVEBOX_PRIMITIVES_H