#include "tfhe.h"
#include "compile_time_settings.h"
#include <tfhe.h>

TFHEClientParams_t makeTFHEClientParams(FILE *secret_key) {
	TFheGateBootstrappingSecretKeySet *key =
	    new_tfheGateBootstrappingSecretKeySet_fromFile(secret_key);
	const TFheGateBootstrappingParameterSet *params = key->params;
	return TFHEClientParams_t{key, params};
}

TFHEServerParams_t makeTFHEServerParams(FILE *cloud_key) {
	TFheGateBootstrappingCloudKeySet *bk =
			new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key);
	const TFheGateBootstrappingParameterSet *params = bk->params;
	return TFHEServerParams_t{params, bk};
}

TFHEServerParams_t makeTFHEServerParams(TFHEClientParams_t p) {
	return TFHEServerParams_t{
		p.key->cloud.params,
		&(p.key->cloud)
	};
}

void freeTFHEServerParams(TFHEServerParams_t p) {
	// Not implemented; see the comment about the const qualifier for bk
	// delete_gate_bootstrapping_cloud_keyset(p.bk);
}
void freeTFHEClientParams(TFHEClientParams_t p) {
	delete_gate_bootstrapping_secret_keyset(p.key);
}

int decrypt(bits_t dst, TFHEClientParams_t p) {
	return bootsSymDecrypt(dst, p.key);
}

bits_t make_bits(int N, const TFheGateBootstrappingParameterSet *params) {
	return new_gate_bootstrapping_ciphertext_array(N, params);
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

void _copy(bits_t dst, bits_t src, TFHEServerParams_t p) {
	bootsCOPY(dst, src, p.bk);
}
