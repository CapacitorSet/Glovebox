#include <cassert>
#include <gsl_span_custom.h>
#include <tfhe.h>
#include "tfhe.h"
#include "compile_time_settings.h"

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

int decrypt(bit_t dst, TFHEClientParams_t p) {
	return bootsSymDecrypt(dst.data(), p.key);
}

bit_t make_bit(TFHEClientParams_t p) {
	return gsl::span<LweSample, 1>(new_gate_bootstrapping_ciphertext(p.params), 1);
}

bit_t make_bit(TFHEServerParams_t p) {
	return gsl::span<LweSample, 1>(new_gate_bootstrapping_ciphertext(p.params), 1);
}

gsl::span<LweSample> make_bitspan(int N, TFHEClientParams_t p) {
	LweSample *ptr = new_gate_bootstrapping_ciphertext_array(N, p.params);
	return gsl::span<LweSample>(ptr, N);
}

gsl::span<LweSample> make_bitspan(int N, TFHEServerParams_t p) {
	LweSample *ptr = new_gate_bootstrapping_ciphertext_array(N, p.params);
	return gsl::span<LweSample>(ptr, N);
}

void constant(bit_t dst, char src, TFHEClientParams_t p) {
	bootsSymEncrypt(dst.data(), src, p.key);
}

void constant(bit_t dst, char src, TFHEServerParams_t p) {
	bootsCONSTANT(dst.data(), src, p.bk);
}

void _not(bit_t dst, bit_t src, TFHEServerParams_t p) {
	bootsNOT(dst.data(), src.data(), p.bk);
}

void _and(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p) {
	bootsAND(dst.data(), a.data(), b.data(), p.bk);
}

void _andyn(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p) {
	bootsANDYN(dst.data(), a.data(), b.data(), p.bk);
}

void _nand(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p) {
	bootsNAND(dst.data(), a.data(), b.data(), p.bk);
}

void _nor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p) {
	bootsNOR(dst.data(), a.data(), b.data(), p.bk);
}

void _xor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p) {
	bootsXOR(dst.data(), a.data(), b.data(), p.bk);
}

void _xnor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p) {
	bootsXNOR(dst.data(), a.data(), b.data(), p.bk);
}

void _or(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p) {
	bootsOR(dst.data(), a.data(), b.data(), p.bk);
}

void free_bitspan(bitspan_t item) {
	free_LweSample_array(item.size(), item.data());
}

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b, TFHEServerParams_t p) {
	bootsMUX(dst.data(), cond.data(), a.data(), b.data(), p.bk);
}

void _copy(bit_t dst, bit_t src, TFHEServerParams_t p) {
	bootsCOPY(dst.data(), src.data(), p.bk);
}
void _copy(bitspan_t dst, bitspan_t src, TFHEServerParams_t p) {
	assert(dst.size() == src.size());
	for (int i = 0; i < dst.size(); i++)
		bootsCOPY(&dst.at(i), &src.at(i), p.bk);
}
