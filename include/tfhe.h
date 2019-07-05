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

extern ServerParams default_server_params;
extern ClientParams default_client_params;
extern WeakParams default_weak_params;

bit_t make_bit(WeakParams = default_weak_params);
bitspan_t make_bitspan(int N, WeakParams = default_weak_params);

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
template <uint8_t size>
fixed_bitspan_t<size> make_bitspan(WeakParams p = default_weak_params) {
	// Can this be rewritten in terms of make_bitspan, subspan?
#if PLAINTEXT
	(void)p;
	auto cptr = reinterpret_cast<bool *>(malloc(size));
#else
	LweSample *cptr = new_gate_bootstrapping_ciphertext_array(size, p.params);
#endif
	auto ptr = gsl::shared_ptr_unsynchronized<unsafe_bit_t>(cptr);
	auto span = gsl::span<size>(ptr, size);
	return fixed_bitspan_t<size>(span);
}

#if PLAINTEXT
// Enable one to write just `decrypt(bit)`, since the client params aren't
// used anyway
int decrypt(bit_t dst, ClientParams p = {});
#else
int decrypt(bit_t dst, ClientParams p = default_client_params);
#endif

// Forbids client code from using constant() unless explicitly overridden
#ifndef STRICT_CLIENT_MODE
#define STRICT_CLIENT_MODE 0
#endif
void _unsafe_constant(bit_t dst, bool src, WeakParams p);

#if STRICT_CLIENT_MODE
static void inline constant(bit_t, bool, ServerParams) {
	fprintf(stderr, "constant() was called in strict client mode - exiting.\n");
	abort();
}
#else
static void inline constant(bit_t dst, bool src,
                            ServerParams p = default_server_params) {
	_unsafe_constant(dst, src, p);
}
#endif
void encrypt(bit_t dst, bool src, ClientParams p = default_client_params);

void _not(bit_t dst, bit_t a, WeakParams = default_weak_params);
void _and(bit_t dst, bit_t a, bit_t b, WeakParams = default_weak_params);
void _andyn(bit_t dst, bit_t a, bit_t b, WeakParams = default_weak_params);
void _andny(bit_t dst, bit_t a, bit_t b, WeakParams = default_weak_params);
void _nand(bit_t dst, bit_t a, bit_t b, WeakParams = default_weak_params);
void _or(bit_t dst, bit_t a, bit_t b, WeakParams = default_weak_params);
void _oryn(bit_t dst, bit_t a, bit_t b, WeakParams = default_weak_params);
void _orny(bit_t dst, bit_t a, bit_t b, WeakParams = default_weak_params);
void _nor(bit_t dst, bit_t a, bit_t b, WeakParams = default_weak_params);
void _xor(bit_t dst, bit_t a, bit_t b, WeakParams = default_weak_params);
void _xnor(bit_t dst, bit_t a, bit_t b, WeakParams = default_weak_params);

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b,
          WeakParams = default_weak_params);
void _copy(bit_t dst, bit_t src, WeakParams = default_weak_params);

void add(bitspan_t result, bitspan_t a, bitspan_t b,
         WeakParams = default_weak_params);
void mult(bitspan_t result, bitspan_t a, bitspan_t b,
          WeakParams = default_weak_params);
#endif // GLOVEBOX_TFHE_H