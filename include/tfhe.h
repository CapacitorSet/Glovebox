#ifndef GLOVEBOX_TFHE_H
#define GLOVEBOX_TFHE_H

#include <cassert>
#include <cstdint>
#include <cstdio>

#if PLAINTEXT
using unsafe_bit_t = bool;
using ClientKey = bool *;
using ServerKey = bool *;

class WeakParams {
  public:
	WeakParams() = default;
};

class ClientParams : public WeakParams {
  public:
	ClientParams() = default;
	ClientParams(ClientKey);
};

class ServerParams : public WeakParams {
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

class WeakParams {
  public:
	const TFheGateBootstrappingParameterSet *params;
	const TFheGateBootstrappingCloudKeySet *cloudKeySet;

	WeakParams() = default;

  protected:
	WeakParams(const TFheGateBootstrappingParameterSet *params,
	           const TFheGateBootstrappingCloudKeySet *cloudKeySet)
	    : params(params), cloudKeySet(cloudKeySet){};
};

class ClientParams : public WeakParams {
  public:
	const TFheGateBootstrappingSecretKeySet *secretKeySet;

	ClientParams() = default;
	ClientParams(ClientKey);
};

class ServerParams : public WeakParams {
  public:
	ServerParams() = default;
	ServerParams(ServerKey);
	// Used very rarely - only in tests for now
	explicit ServerParams(const ClientParams &src);
};
#endif

ClientKey read_client_key(char const *filename);
ClientKey read_client_key(FILE *);
void free_client_key(ClientKey);

ServerKey read_server_key(char const *filename);
ServerKey read_server_key(FILE *);
void free_server_key(ServerKey);

#include <gsl_span_custom.h>
typedef gsl::span<> bitspan_t;
typedef gsl::span<1> bit_t;

#if GB_SERVER
extern thread_local ServerParams server_params;
#else
extern thread_local ClientParams client_params;
#endif
extern thread_local WeakParams weak_params;

enum class ModePicker { CLIENT, SERVER };
#define DefaultMode (GB_SERVER ? ModePicker::SERVER : ModePicker::CLIENT)

bit_t make_bit();
bitspan_t make_bitspan(int N);

template <uint8_t size> class fixed_bitspan_t : public gsl::span<size> {
  public:
	// Seemingly required to instance the inner span
	explicit fixed_bitspan_t(gsl::span<size> span) : gsl::span<size>(span){};
	// Checks at runtime that this conversion is possible.
	// This operator is explicit so that we can do semmingly-unsafe conversions
	// anyway
	explicit fixed_bitspan_t(gsl::span<> span) : gsl::span<size>(span) {
		assert(span.size() == size);
	};
};
template <uint8_t size> fixed_bitspan_t<size> make_bitspan() {
	// Can this be rewritten in terms of make_bitspan, subspan?
#if PLAINTEXT
	auto cptr = reinterpret_cast<bool *>(malloc(size));
#else
	LweSample *cptr = new_gate_bootstrapping_ciphertext_array(size, weak_params.params);
#endif
	auto ptr = gsl::shared_ptr_unsynchronized<unsafe_bit_t>(cptr);
	auto span = gsl::span<size>(ptr, size);
	return fixed_bitspan_t<size>(span);
}

int decrypt(bit_t dst);

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
#endif
void encrypt(bit_t dst, bool src);

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

void add(bitspan_t result, bitspan_t a, bitspan_t b);
void mult(bitspan_t result, bitspan_t a, bitspan_t b);
#endif // GLOVEBOX_TFHE_H