#ifndef FHE_CPU_TFHE_H
#define FHE_CPU_TFHE_H

#include "compile_time_settings.h"
#include <cstdint>
#include <cstdio>
#include <gsl_span_custom.h>

#include <tfhe/tfhe.h>
typedef gsl::span<LweSample> bitspan_t;
typedef gsl::span<LweSample, 1> bit_t;
typedef struct {
	const TFheGateBootstrappingParameterSet *params;
	/* Todo: remove the const qualifier, deal with the issue that TFHEServerParams_t can be created either from a file
	 * (resulting in `TFHEGateBoostrappingCloudKeySet*`) or from TFHEClientParams_t (resulting in `const TFHEGate...Set*`).
	 */
	const TFheGateBootstrappingCloudKeySet *bk;
} TFHEServerParams_t;
typedef struct {
	TFheGateBootstrappingSecretKeySet* key;
	const TFheGateBootstrappingParameterSet* params;
} TFHEClientParams_t;

TFHEClientParams_t makeTFHEClientParams(FILE *secret_key);
TFHEServerParams_t makeTFHEServerParams(FILE *cloud_key);
TFHEServerParams_t makeTFHEServerParams(TFHEClientParams_t p);
void freeTFHEServerParams(TFHEServerParams_t p);
void freeTFHEClientParams(TFHEClientParams_t p);

bit_t make_bit(TFHEServerParams_t p);
bit_t make_bit(TFHEClientParams_t p);
gsl::span<LweSample> make_bitspan(int N, TFHEServerParams_t p);
gsl::span<LweSample> make_bitspan(int N, TFHEServerParams_t p);

void free_bitspan(bitspan_t item);

int decrypt(bit_t dst, TFHEClientParams_t p);

void constant(bit_t dst, char src, TFHEServerParams_t p);
void constant(bit_t dst, char src, TFHEClientParams_t p);

void _not(bit_t dst, bit_t a, TFHEServerParams_t p);
void _and(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p);
void _andyn(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p);
void _andny(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p);
void _nand(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p);
void _or(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p);
void _nor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p);
void _xor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p);
void _nxor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p);

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b, TFHEServerParams_t p);
void _copy(bit_t dst, bit_t src, TFHEServerParams_t p);
void _copy(bitspan_t dst, bitspan_t src, TFHEServerParams_t p);
#endif // FHE_CPU_TFHE_H