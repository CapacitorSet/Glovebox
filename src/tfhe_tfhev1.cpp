#include <tfhe.h>

TFHEServerParams_t unwrap_only(only_TFHEServerParams_t p) {
	return TFHEServerParams_t{p.params, p.bk};
};

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
	return TFHEServerParams_t{p.key->cloud.params, &(p.key->cloud)};
}

void freeTFHEServerParams(TFHEServerParams_t p) {
	// Not implemented; see the comment about the const qualifier for bk
	// delete_gate_bootstrapping_cloud_keyset(p.bk);
}
void freeTFHEClientParams(TFHEClientParams_t p) {
	delete_gate_bootstrapping_secret_keyset(p.key);
}

int decrypt(bit_t dst, TFHEClientParams_t p) {
	return bootsSymDecrypt(dst.cptr(), p.key);
}

bit_t make_bit(TFHEClientParams_t p) {
	LweSample *cptr = new_gate_bootstrapping_ciphertext(p.params);
	auto ptr = std::shared_ptr<LweSample>(cptr);
	return gsl::span<LweSample, 1>(ptr, 1);
}

bit_t make_bit(TFHEServerParams_t p) {
	LweSample *cptr = new_gate_bootstrapping_ciphertext(p.params);
	auto ptr = std::shared_ptr<LweSample>(cptr);
	return gsl::span<LweSample, 1>(ptr, 1);
}

bitspan_t make_bitspan(int N, TFHEClientParams_t p) {
	LweSample *cptr = new_gate_bootstrapping_ciphertext(p.params);
	auto ptr = std::shared_ptr<LweSample>(cptr);
	return gsl::span<LweSample>(ptr, N);
}

bitspan_t make_bitspan(int N, TFHEServerParams_t p) {
	LweSample *cptr = new_gate_bootstrapping_ciphertext_array(N, p.params);
	auto ptr = std::shared_ptr<LweSample>(cptr);
	return gsl::span<LweSample>(ptr, N);
}

void encrypt(bit_t dst, bool src, TFHEClientParams_t p) {
	bootsSymEncrypt(dst.cptr(), src, p.key);
}

void _internal_constant(bit_t dst, bool src, only_TFHEServerParams_t p) {
	bootsCONSTANT(dst.cptr(), src, p.bk);
};

void _not(bit_t dst, bit_t src, TFHEServerParams_t p) {
	bootsNOT(dst.cptr(), src.cptr(), p.bk);
}

#define BINARY_OPERATOR(LibName, TFHEName)                                     \
	void _##LibName(bit_t dst, const bit_t a, const bit_t b,                   \
	                TFHEServerParams_t p) {                                    \
		boots##TFHEName(dst.cptr(), a.cptr(), b.cptr(), p.bk);                 \
	}

BINARY_OPERATOR(and, AND)
BINARY_OPERATOR(andyn, ANDYN)
BINARY_OPERATOR(andny, ANDNY)
BINARY_OPERATOR(nand, NAND)
BINARY_OPERATOR(or, OR)
BINARY_OPERATOR(oryn, ORYN)
BINARY_OPERATOR(orny, ORNY)
BINARY_OPERATOR(nor, NOR)
BINARY_OPERATOR(xor, XOR)
BINARY_OPERATOR(xnor, XNOR)

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b, TFHEServerParams_t p) {
	bootsMUX(dst.cptr(), cond.cptr(), a.cptr(), b.cptr(), p.bk);
}

void _copy(bit_t dst, bit_t src, TFHEServerParams_t p) {
	bootsCOPY(dst.cptr(), src.cptr(), p.bk);
}