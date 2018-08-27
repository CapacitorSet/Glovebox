#include "compile_time_settings.h"
#include "tfhe.h"

#if IS_PLAINTEXT
#include <malloc.h>
#include <assert.h>

TFHEParams makeTFHEParams(FILE *cloud_key) {
	return TFHEParams{};
}
void freeTFHEParams(TFHEParams p) {}

bits_t make_bits(int N, TFHEParams p) {
	return (char*) malloc(N);
}

bits_t make_bits(int N) {
	assert(false);
	return (char*) malloc(N);
}

void constant(bits_t dst, char src, TFHEParams p) {
	*dst = src;
}

void _not(bits_t dst, bits_t src, TFHEParams p) {
	*dst = !*src;
}

void _and(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	*dst = *a && *b;
}

void _andyn(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	*dst = *a && !*b;
}

void _andny(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	*dst = !*a && *b;
}

void _nand(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	*dst = !(*a && *b);
}

void _or(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	*dst = *a || *b;
}

void _nor(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	*dst = !(*a || *b);
}

void _xor(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	*dst = *a ^ *b;
}

void free_bits(bits_t item) {
	free(item);
}

void _mux(bits_t dst, bits_t cond, bits_t a, bits_t b, TFHEParams p) {
	*dst = *cond ? *a : *b;
}

void _copy(bits_t dst, bits_t src, TFHEParams p) {
	*dst = *src;
}
#else

TFHEParams makeTFHEParams(FILE *cloud_key) {
	TFheGateBootstrappingCloudKeySet *bk = new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key);
	const TFheGateBootstrappingParameterSet *params = bk->params;
	return TFHEParams{params, bk};
}
void freeTFHEParams(TFHEParams p) {
	delete_gate_bootstrapping_cloud_keyset(p.bk);
	delete_gate_bootstrapping_parameters(p.params);
#if DEBUG
	delete_gate_bootstrapping_secret_keyset(p.secret_key);
#endif
}


bits_t make_bits(int N, TFHEParams p) {
	return new_gate_bootstrapping_ciphertext_array(N, p.params);
}

void constant(bits_t dst, char src, TFHEParams p) {
	bootsCONSTANT(dst, src, p.bk);
}

void _and(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	bootsAND(dst, a, b, p.bk);
}

void _andyn(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	bootsANDYN(dst, a, b, p.bk);
}

void _nor(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	bootsNOR(dst, a, b, p.bk);
}

void _xor(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	bootsXOR(dst, a, b, p.bk);
}

void _or(bits_t dst, bits_t a, bits_t b, TFHEParams p) {
	bootsOR(dst, a, b, p.bk);
}

void free_bits(bits_t item) {
	free_LweSample(item);
}

void _mux(bits_t dst, bits_t cond, bits_t a, bits_t b, TFHEParams p) {
	bootsMUX(dst, cond, a, b, p.bk);
}

void _copy(bits_t dst, bits_t src, TFHEParams p) {
	bootsCOPY(dst, src, p.bk);
}
#endif