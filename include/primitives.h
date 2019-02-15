#ifndef FHETOOLS_PRIMITIVES_H
#define FHETOOLS_PRIMITIVES_H

#include <tfhe.h>

void zero(bitspan_t src, TFHEClientParams_t p);
void zero(bitspan_t src, TFHEServerParams_t p = default_server_params);

// No bounds checking is done!
void _memcpy(bitspan_t dst, bitspan_t src, size_t size, TFHEServerParams_t p = default_server_params);
void _copy(bitspan_t dst, bitspan_t src, TFHEServerParams_t p = default_server_params);

// Warning: unlike with the full adder, A and B must not be the same variables as out
void _half_adder(bit_t out, bit_t carry_out, bit_t A, bit_t B, TFHEServerParams_t p = default_server_params);
void _full_adder(bit_t out, bit_t carry_out, bit_t A, bit_t B, bit_t carry_in, TFHEServerParams_t p = default_server_params);

void add8(bit_t carry, bitspan_t out, const bitspan_t a, const bitspan_t b, TFHEServerParams_t p);
void add16(bit_t carry, bitspan_t out, const bitspan_t a, const bitspan_t b, TFHEServerParams_t p);
void mul8(bitspan_t out, const bitspan_t a, const bitspan_t b, TFHEServerParams_t p);
void mul16(bitspan_t out, const bitspan_t a, const bitspan_t b, TFHEServerParams_t p);

#endif //FHETOOLS_PRIMITIVES_H