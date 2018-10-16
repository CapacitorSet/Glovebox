#ifndef FHE_CPU_TFHE_H
#define FHE_CPU_TFHE_H

#include "compile_time_settings.h"
#include <cstdint>
#include <cstdio>

#include <tfhe/tfhe.h>
typedef LweSample *bits_t;
typedef struct {
	const TFheGateBootstrappingParameterSet *params;
	TFheGateBootstrappingCloudKeySet *bk;
} TFHEServerParams_t;
typedef struct {
	TFheGateBootstrappingSecretKeySet* key;
	const TFheGateBootstrappingParameterSet* params;
} TFHEClientParams_t;

TFHEServerParams_t makeTFHEServerParams(FILE *cloud_key);
TFHEClientParams_t makeTFHEClientParams(FILE *secret_key);
void freeTFHEServerParams(TFHEServerParams_t p);
void freeTFHEClientParams(TFHEClientParams_t p);

bits_t make_bits(int N, const TFheGateBootstrappingParameterSet *params);
bits_t make_bits(int N, TFHEServerParams_t p);
bits_t make_bits(int N, TFHEClientParams_t p);
void free_bits(bits_t item);

int decrypt(bits_t dst, TFHEClientParams_t p);

void constant(bits_t dst, char src, TFHEServerParams_t p);
void constant(bits_t dst, char src, TFHEClientParams_t p);

void _not(bits_t dst, bits_t a, TFHEServerParams_t p);
void _and(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p);
void _andyn(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p);
void _andny(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p);
void _nand(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p);
void _or(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p);
void _nor(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p);
void _xor(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p);
void _nxor(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p);

void _mux(bits_t dst, bits_t cond, bits_t a, bits_t b, TFHEServerParams_t p);
void _copy(bits_t dst, bits_t src, TFHEServerParams_t p);
#endif // FHE_CPU_TFHE_H