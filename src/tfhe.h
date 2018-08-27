#ifndef FHE_CPU_TFHE_H
#define FHE_CPU_TFHE_H

#include <cstdint>
#include <cstdio>
#include "compile_time_settings.h"

#if IS_PLAINTEXT
	typedef char* bits_t;

	typedef struct {
	} TFHEParams;
#else
	#include <tfhe/tfhe.h>
	typedef LweSample* bits_t;
	typedef struct {
		const TFheGateBootstrappingParameterSet* params;
		TFheGateBootstrappingCloudKeySet* bk;
		#if DEBUG
			TFheGateBootstrappingSecretKeySet* secret_key;
		#endif
	} TFHEParams;
#endif

TFHEParams makeTFHEParams(FILE *cloud_key);
void freeTFHEParams(TFHEParams p);

bits_t make_bits(int N, TFHEParams p);
void free_bits(bits_t item);

void constant(bits_t dst, char src, TFHEParams p);

void _not(bits_t dst, bits_t a, TFHEParams p);
void _and(bits_t dst, bits_t a, bits_t b, TFHEParams p);
void _andyn(bits_t dst, bits_t a, bits_t b, TFHEParams p);
void _andny(bits_t dst, bits_t a, bits_t b, TFHEParams p);
void _nand(bits_t dst, bits_t a, bits_t b, TFHEParams p);
void _or(bits_t dst, bits_t a, bits_t b, TFHEParams p);
void _nor(bits_t dst, bits_t a, bits_t b, TFHEParams p);
void _xor(bits_t dst, bits_t a, bits_t b, TFHEParams p);
void _nxor(bits_t dst, bits_t a, bits_t b, TFHEParams p);

void _mux(bits_t dst, bits_t cond, bits_t a, bits_t b, TFHEParams p);
void _copy(bits_t dst, bits_t src, TFHEParams p);
#endif //FHE_CPU_TFHE_H