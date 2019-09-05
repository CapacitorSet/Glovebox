#ifndef GLOVEBOX_TFHE_H
#define GLOVEBOX_TFHE_H

#include <cassert>
#include <cstdint>
#include <cstdio>

#if PLAINTEXT
using unsafe_bit_t = bool;
using ClientKey = bool *;
using ServerKey = bool *;

class ClientParams {
  public:
	ClientParams() = default;
	ClientParams(ClientKey);
};

class ServerParams {
  public:
	ServerParams() = default;
	ServerParams(ServerKey);
	// Used very rarely - only in tests for now
	explicit ServerParams(const ClientParams &src);
};

#else
#include <tfhe/tfhe.h>
using unsafe_bit_t = LweSample;

using ClientKey = TFheGateBootstrappingSecretKeySet *;
using ServerKey = TFheGateBootstrappingCloudKeySet *;

class ClientParams {
  public:
	const TFheGateBootstrappingParameterSet *params;
	const TFheGateBootstrappingCloudKeySet *cloudKeySet;
	const TFheGateBootstrappingSecretKeySet *secretKeySet;

	ClientParams() = default;
	ClientParams(ClientKey);
};

class ServerParams {
  public:
	const TFheGateBootstrappingParameterSet *params;
	const TFheGateBootstrappingCloudKeySet *cloudKeySet;

	ServerParams() = default;
	ServerParams(ServerKey);
	// Used very rarely - only in tests for now
	explicit ServerParams(const ClientParams &src);
};
#endif

#if GB_SERVER
extern thread_local ServerParams server_params;
#define weak_params server_params
#else
extern thread_local ClientParams client_params;
#define weak_params client_params
#endif

ClientKey read_client_key(char const *filename);
ClientKey read_client_key(FILE *);
void free_client_key(ClientKey);

ServerKey read_server_key(char const *filename);
ServerKey read_server_key(FILE *);
void free_server_key(ServerKey);

#include <bitvec.h>
using bitvec_t = gb::bitvec<>;
using bit_t = gb::bitvec<1>;

bit_t make_bit();
bitvec_t make_bitvec(int N);

// Todo: replace with bitvec<N>?
template <uint8_t size> class fixed_bitvec_t : public gb::bitvec<size> {
  public:
	explicit fixed_bitvec_t(gb::bitvec<size> span) : gb::bitvec<size>(span){};
	// Checks at runtime that this conversion is possible.
	// This operator is explicit so that we can do semmingly-unsafe conversions
	// anyway
	explicit fixed_bitvec_t(gb::bitvec<> span) : gb::bitvec<size>(span) {
		assert(span.size() == size);
	};
};
template <uint8_t size> fixed_bitvec_t<size> make_bitvec() {
	// Can this be rewritten in terms of make_bitvec, subspan?
#if PLAINTEXT
	auto cptr = reinterpret_cast<bool *>(malloc(size));
#else
	LweSample *cptr = new_gate_bootstrapping_ciphertext_array(size, weak_params.params);
#endif
	auto ptr = gb::shared_ptr_unsynchronized<unsafe_bit_t>(cptr);
	auto span = gb::bitvec<size>(ptr, size);
	return fixed_bitvec_t<size>(span);
}

void _unsafe_constant(bit_t dst, bool src);
#if GB_SERVER
static void inline constant(bit_t dst, bool src) {
	_unsafe_constant(dst, src);
}
#else
static void inline constant(bit_t, bool) {
	fprintf(stderr, "constant() was called in client mode - exiting.\n");
	abort();
}
void encrypt(bit_t dst, bool src);
int decrypt(bit_t dst);
#endif

static void inline write(bit_t dst, bool src) {
#if GB_SERVER
	_unsafe_constant(dst, src);
#else
	encrypt(dst, src);
#endif
}

void _not(bit_t dst, bit_t a);
void _and(bit_t dst, bit_t a, bit_t b);
void _andyn(bit_t dst, bit_t a, bit_t b);
void _andny(bit_t dst, bit_t a, bit_t b);
void _nand(bit_t dst, bit_t a, bit_t b);
void _or(bit_t dst, bit_t a, bit_t b);
void _oryn(bit_t dst, bit_t a, bit_t b);
void _orny(bit_t dst, bit_t a, bit_t b);
void _nor(bit_t dst, bit_t a, bit_t b);
void _xor(bit_t dst, bit_t a, bit_t b);
void _xnor(bit_t dst, bit_t a, bit_t b);

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b);
void _copy(bit_t dst, bit_t src);

void add(bitvec_t result, bitvec_t a, bitvec_t b);
void mult(bitvec_t result, bitvec_t a, bitvec_t b);
#endif // GLOVEBOX_TFHE_H