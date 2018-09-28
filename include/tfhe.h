#ifndef FHE_CPU_TFHE_H
#define FHE_CPU_TFHE_H

#include "compile_time_settings.h"
#include <cstdint>
#include <cstdio>

#if IS_PLAINTEXT
typedef char *bits_t;

typedef struct {
} TFHEParams;
#else
#include <tfhe/tfhe.h>
typedef LweSample *bits_t;
typedef struct {
	const TFheGateBootstrappingParameterSet *params;
	TFheGateBootstrappingCloudKeySet *bk;
#if DEBUG
	TFheGateBootstrappingSecretKeySet *secret_key;
#endif
} TFHEServerParams_t;
typedef struct {
	TFheGateBootstrappingSecretKeySet* key;
	const TFheGateBootstrappingParameterSet* params;
} TFHEClientParams_t;
#endif

TFHEServerParams_t makeTFHEServerParams(FILE *cloud_key);
TFHEClientParams_t makeTFHEClientParams(FILE *secret_key);
void freeTFHEServerParams(TFHEServerParams_t p);

bits_t make_bits(int N, TFHEServerParams_t p);
bits_t make_bits(int N, TFHEClientParams_t p);
void free_bits(bits_t item);

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