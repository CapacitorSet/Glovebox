#include <tfhe.h>

ClientKey read_client_key(char const *filename) {
	FILE *file = fopen(filename, "rb");
	if (file == nullptr)
		return nullptr;
	ClientKey ret = read_client_key(file);
	fclose(file);
	return ret;
}
ClientKey read_client_key(FILE *file) {
	return new_tfheGateBootstrappingSecretKeySet_fromFile(file);
}
void free_client_key(ClientKey key) {
	delete_gate_bootstrapping_secret_keyset(key);
}

ServerKey read_server_key(char const *filename) {
	FILE *file = fopen(filename, "rb");
	if (file == nullptr)
		return nullptr;
	ServerKey ret = read_server_key(file);
	fclose(file);
	return ret;
}
ServerKey read_server_key(FILE *file) {
	return new_tfheGateBootstrappingCloudKeySet_fromFile(file);
}
void free_server_key(ServerKey key) {
	delete_gate_bootstrapping_cloud_keyset(key);
}

ClientParams::ClientParams(ClientKey client_key)
    : WeakParams(client_key->params, &client_key->cloud),
      secretKeySet(client_key){};

ServerParams::ServerParams(ServerKey cloud_key)
    : WeakParams(cloud_key->params, cloud_key) {}

ServerParams::ServerParams(const ClientParams &src) : WeakParams(src) {}

int decrypt(bit_t dst, ClientParams p) {
	return bootsSymDecrypt(dst.cptr(), p.secretKeySet);
}

bit_t make_bit(WeakParams p) {
	LweSample *cptr = new_gate_bootstrapping_ciphertext(p.params);
	auto ptr = gsl::shared_ptr_unsynchronized<LweSample>(cptr);
	return gsl::span<1>(ptr, 1);
}

bitspan_t make_bitspan(int N, WeakParams p) {
	LweSample *cptr = new_gate_bootstrapping_ciphertext_array(N, p.params);
	auto ptr = gsl::shared_ptr_unsynchronized<LweSample>(cptr);
	return gsl::span<>(ptr, N);
}

void encrypt(bit_t dst, bool src, ClientParams p) {
	bootsSymEncrypt(dst.cptr(), src, p.secretKeySet);
}

void _unsafe_constant(bit_t dst, bool src, WeakParams p) {
	bootsCONSTANT(dst.cptr(), src, p.cloudKeySet);
};

void _not(bit_t dst, bit_t src, WeakParams p) {
	bootsNOT(dst.cptr(), src.cptr(), p.cloudKeySet);
}

#define BINARY_OPERATOR(LibName, TFHEName)                                     \
	void _##LibName(bit_t dst, const bit_t a, const bit_t b, WeakParams p) {   \
		boots##TFHEName(dst.cptr(), a.cptr(), b.cptr(), p.cloudKeySet);        \
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

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b, WeakParams p) {
	bootsMUX(dst.cptr(), cond.cptr(), a.cptr(), b.cptr(), p.cloudKeySet);
}

void _copy(bit_t dst, bit_t src, WeakParams p) {
	bootsCOPY(dst.cptr(), src.cptr(), p.cloudKeySet);
}
