#include <tfhe.h>
#include "tfhe.h"
#include "compile_time_settings.h"

#if IS_PLAINTEXT
#include <assert.h>
#include <malloc.h>

TFHEServerParams_t makeTFHEServerParams_t(FILE *cloud_key) { return TFHEServerParams_t{}; }
void freeTFHEServerParams_t(TFHEServerParams_t p) {}

bits_t make_bits(int N, TFHEServerParams_t p) { return (char *)malloc(N); }

bits_t make_bits(int N) {
  assert(false);
  return (char *)malloc(N);
}

void constant(bits_t dst, char src, TFHEServerParams_t p) { *dst = src; }

void _not(bits_t dst, bits_t src, TFHEServerParams_t p) { *dst = !*src; }

void _and(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) { *dst = *a && *b; }

void _andyn(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) { *dst = *a && !*b; }

void _andny(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) { *dst = !*a && *b; }

void _nand(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) { *dst = !(*a && *b); }

void _or(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) { *dst = *a || *b; }

void _nor(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) { *dst = !(*a || *b); }

void _xor(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) { *dst = *a ^ *b; }

void free_bits(bits_t item) { free(item); }

void _mux(bits_t dst, bits_t cond, bits_t a, bits_t b, TFHEServerParams_t p) {
  *dst = *cond ? *a : *b;
}

void _copy(bits_t dst, bits_t src, TFHEServerParams_t p) { *dst = *src; }
#else

TFHEServerParams_t makeTFHEServerParams(FILE *cloud_key) {
  TFheGateBootstrappingCloudKeySet *bk =
      new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key);
  const TFheGateBootstrappingParameterSet *params = bk->params;
  return TFHEServerParams_t{params, bk};
}
TFHEClientParams_t makeTFHEClientParams(FILE *secret_key) {
	TFheGateBootstrappingSecretKeySet* key = new_tfheGateBootstrappingSecretKeySet_fromFile(secret_key);
	const TFheGateBootstrappingParameterSet* params = key->params;
	return TFHEClientParams_t{key, params};
}

void freeTFHEServerParams(TFHEServerParams_t p) {
  delete_gate_bootstrapping_cloud_keyset(p.bk);
  // Seems to be not needed
  // delete_gate_bootstrapping_parameters(p.params);
#if DEBUG
  delete_gate_bootstrapping_secret_keyset(p.secret_key);
#endif
}

bits_t make_bits(int N, TFHEClientParams_t p) {
	return new_gate_bootstrapping_ciphertext_array(N, p.params);
}

bits_t make_bits(int N, TFHEServerParams_t p) {
  return new_gate_bootstrapping_ciphertext_array(N, p.params);
}

void constant(bits_t dst, char src, TFHEClientParams_t p) {
	bootsSymEncrypt(dst, src, p.key);
}

void constant(bits_t dst, char src, TFHEServerParams_t p) {
	bootsCONSTANT(dst, src, p.bk);
}

void _and(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) {
  bootsAND(dst, a, b, p.bk);
}

void _andyn(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) {
  bootsANDYN(dst, a, b, p.bk);
}

void _nor(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) {
  bootsNOR(dst, a, b, p.bk);
}

void _xor(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) {
  bootsXOR(dst, a, b, p.bk);
}

void _or(bits_t dst, bits_t a, bits_t b, TFHEServerParams_t p) {
  bootsOR(dst, a, b, p.bk);
}

void free_bits(bits_t item) { free_LweSample(item); }

void _mux(bits_t dst, bits_t cond, bits_t a, bits_t b, TFHEServerParams_t p) {
  bootsMUX(dst, cond, a, b, p.bk);
}

void _copy(bits_t dst, bits_t src, TFHEServerParams_t p) { bootsCOPY(dst, src, p.bk); }
#endif