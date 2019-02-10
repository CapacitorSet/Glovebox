#include <cassert>
#include "fhe-tools.h"

/* An "agnostic" interface to TFHE. Might support a plaintext backend and TFHE v2 in the future.
 */

#if PLAINTEXT
#warning You are compiling a plaintext binary.

TFHEClientParams_t makeTFHEClientParams(FILE*) {
	return TFHEClientParams_t{};
}

TFHEServerParams_t makeTFHEServerParams(FILE*) {
	return TFHEServerParams_t{};
}

TFHEServerParams_t makeTFHEServerParams(TFHEClientParams_t) {
	return TFHEServerParams_t{};
}

void freeTFHEServerParams(TFHEServerParams_t) {
}
void freeTFHEClientParams(TFHEClientParams_t) {
}

int decrypt(bit_t dst, TFHEClientParams_t) {
	return *dst.data();
}

bit_t make_bit(TFHEClientParams_t) {
	return gsl::span<bool, 1>(reinterpret_cast<bool*>(malloc(1)), 1);
}

bit_t make_bit(TFHEServerParams_t) {
	return gsl::span<bool, 1>(reinterpret_cast<bool*>(malloc(1)), 1);
}

bitspan_t make_bitspan(int N, TFHEClientParams_t) {
	return gsl::span<bool>(reinterpret_cast<bool*>(malloc(N)), N);
}

bitspan_t make_bitspan(int N, TFHEServerParams_t) {
	return gsl::span<bool>(reinterpret_cast<bool*>(malloc(N)), N);
}

void constant(bit_t dst, bool src, TFHEClientParams_t) {
	*dst.data() = src;
}

void constant(bit_t dst, bool src, TFHEServerParams_t) {
	*dst.data() = src;
}

void _not(bit_t dst, bit_t src, TFHEServerParams_t) {
	*dst.data() = !*src.data();
}

void _and(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t) {
	*dst.data() = *a.data() && *b.data();
}

void _andyn(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t) {
	*dst.data() = *a.data() && !*b.data();
}

void _andny(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t) {
	*dst.data() = !*a.data() && *b.data();
}

void _nand(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t) {
	*dst.data() = !(*a.data() && *b.data());
}

void _nor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t) {
	*dst.data() = !(*a.data() || *b.data());
}

void _xor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t) {
	*dst.data() = *a.data() ^ *b.data();
}

void _xnor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t) {
	*dst.data() = !(*a.data() ^ *b.data());
}

void _or(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t) {
	*dst.data() = *a.data() || *b.data();
}

void free_bitspan(bitspan_t item) {
	free(item.data());
}

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b, TFHEServerParams_t) {
	*dst.data() = *cond.data() ? *a.data() : *b.data();
}

void _copy(bit_t dst, bit_t src, TFHEServerParams_t) {
	*dst.data() = *src.data();
}

#else

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

bitspan_t make_bitspan(int N, TFHEClientParams_t p) {
	LweSample *ptr = new_gate_bootstrapping_ciphertext_array(N, p.params);
	return gsl::span<LweSample>(ptr, N);
}

bitspan_t make_bitspan(int N, TFHEServerParams_t p) {
	LweSample *ptr = new_gate_bootstrapping_ciphertext_array(N, p.params);
	return gsl::span<LweSample>(ptr, N);
}

void encrypt(bit_t dst, bool src, TFHEClientParams_t p) {
	bootsSymEncrypt(dst.data(), src, p.key);
}

void constant(bit_t dst, bool src, TFHEServerParams_t p) {
	bootsCONSTANT(dst.data(), src, p.bk);
}

void _not(bit_t dst, bit_t src, TFHEServerParams_t p) {
	bootsNOT(dst.data(), src.data(), p.bk);
}

#define BINARY_OPERATOR(LibName, TFHEName) void _ ## LibName(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p) { \
	boots ## TFHEName(dst.data(), a.data(), b.data(), p.bk); \
}

BINARY_OPERATOR(and, AND)
BINARY_OPERATOR(andyn, ANDYN)
BINARY_OPERATOR(andny, ANDNY)
BINARY_OPERATOR(nand, NAND)
BINARY_OPERATOR(nor, NOR)
BINARY_OPERATOR(xor, XOR)
BINARY_OPERATOR(xnor, XNOR)
BINARY_OPERATOR(or, OR)

void free_bitspan(bitspan_t item) {
	free_LweSample_array(item.size(), item.data());
}

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b, TFHEServerParams_t p) {
	bootsMUX(dst.data(), cond.data(), a.data(), b.data(), p.bk);
}

void _copy(bit_t dst, bit_t src, TFHEServerParams_t p) {
	bootsCOPY(dst.data(), src.data(), p.bk);
}

#endif