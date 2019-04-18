#ifndef FHETOOLS_TFHE_H
#define FHETOOLS_TFHE_H

#include <cassert>
#include <cstdint>
#include <gsl_span_custom.h>
#include <tfhe/tfhe.h>

#if PLAINTEXT
using unsafe_bit_t = bool;
typedef struct {
} only_TFHEServerParams_t;
typedef struct {
	operator only_TFHEServerParams_t() const {
		return only_TFHEServerParams_t{};
	}
} TFHEServerParams_t;
typedef struct {
	operator TFHEServerParams_t() const { return TFHEServerParams_t{}; }
	operator only_TFHEServerParams_t() =
	    delete; // If you can read this you're passing client params to
	            // server-only functions.
} TFHEClientParams_t;
#else
using unsafe_bit_t = LweSample;

typedef struct {
	const TFheGateBootstrappingParameterSet *params;
	/* Todo: remove the const qualifier, deal with the issue that
	 * TFHEServerParams_t can be created either from a file (resulting in
	 * `TFHEGateBoostrappingCloudKeySet*`) or from TFHEClientParams_t (resulting
	 * in `const TFHEGate...Set*`).
	 */
	const TFheGateBootstrappingCloudKeySet *bk;
} only_TFHEServerParams_t;
typedef struct {
	const TFheGateBootstrappingParameterSet *params;
	const TFheGateBootstrappingCloudKeySet *bk;
	operator only_TFHEServerParams_t() const {
		return only_TFHEServerParams_t{params, bk};
	}
} TFHEServerParams_t;
typedef struct {
	TFheGateBootstrappingSecretKeySet *key;
	const TFheGateBootstrappingParameterSet *params;
	operator TFHEServerParams_t() const {
		return TFHEServerParams_t{key->cloud.params, &(key->cloud)};
	}
	operator only_TFHEServerParams_t() =
	    delete; // If you can read this you're passing client params to
	            // server-only functions.
} TFHEClientParams_t;
#endif

typedef gsl::span<unsafe_bit_t> bitspan_t;
typedef gsl::span<unsafe_bit_t, 1> bit_t;

// C++ doesn't allow us to define a conversion operator for only_TFHESrvParam to
// TFHESrvParam.
TFHEServerParams_t unwrap_only(only_TFHEServerParams_t p);
TFHEClientParams_t makeTFHEClientParams(FILE *secret_key);
TFHEServerParams_t makeTFHEServerParams(FILE *cloud_key);
void freeTFHEServerParams(only_TFHEServerParams_t p);
void freeTFHEClientParams(TFHEClientParams_t p);

extern TFHEServerParams_t default_server_params;
extern TFHEClientParams_t default_client_params;
bit_t make_bit(TFHEServerParams_t = default_server_params);
bit_t make_bit(TFHEClientParams_t p);
bitspan_t make_bitspan(int N, TFHEClientParams_t p);
bitspan_t make_bitspan(int N, TFHEServerParams_t = default_server_params);

template <uint8_t size>
class fixed_bitspan_t : public gsl::span<unsafe_bit_t, size> {
  public:
	// Seemingly required to instance the inner span
	explicit fixed_bitspan_t(gsl::span<unsafe_bit_t, size> span)
	    : gsl::span<unsafe_bit_t, size>(span){};
	// Checks at runtime that this conversion is possible.
	// This operator is explicit so that we can do semmingly-unsafe conversions
	// anyway
	explicit fixed_bitspan_t(gsl::span<unsafe_bit_t> span)
	    : gsl::span<unsafe_bit_t, size>(span) {
		assert(span.size() == size);
	};
};
template <uint8_t size>
fixed_bitspan_t<size>
make_bitspan(TFHEServerParams_t p = default_server_params) {
	// Can this be rewritten in terms of make_bitspan, subspan?
#if PLAINTEXT
	(void)p;
	auto cptr = reinterpret_cast<bool *>(malloc(size));
#else
	LweSample *cptr = new_gate_bootstrapping_ciphertext_array(size, p.params);
#endif
	auto ptr = std::shared_ptr<unsafe_bit_t>(cptr);
	auto span = gsl::span<unsafe_bit_t, size>(ptr, size);
	return fixed_bitspan_t<size>(span);
}

#if PLAINTEXT
// Enable one to write just `decrypt(bit)`, since the client params aren't
// used anyway
int decrypt(bit_t dst, TFHEClientParams_t p = {});
#else
int decrypt(bit_t dst, TFHEClientParams_t p = default_client_params);
#endif

// Forbids client code from using constant() unless explicitly overridden
#ifndef STRICT_CLIENT_MODE
#define STRICT_CLIENT_MODE 0
#endif
void _internal_constant(bit_t dst, bool src, only_TFHEServerParams_t);
template <bool override_strict_client_mode = false>
static void inline constant(bit_t dst, bool src,
                            only_TFHEServerParams_t p = default_server_params) {
	if constexpr (STRICT_CLIENT_MODE && !override_strict_client_mode) {
		fprintf(stderr,
		        "constant() was called in strict client mode - exiting.\n");
		abort();
	} else {
		_internal_constant(dst, src, p);
	}
}
void encrypt(bit_t dst, bool src, TFHEClientParams_t p = default_client_params);

void _not(bit_t dst, bit_t a, TFHEServerParams_t = default_server_params);
void _and(bit_t dst, bit_t a, bit_t b,
          TFHEServerParams_t = default_server_params);
void _andyn(bit_t dst, bit_t a, bit_t b,
            TFHEServerParams_t = default_server_params);
void _andny(bit_t dst, bit_t a, bit_t b,
            TFHEServerParams_t = default_server_params);
void _nand(bit_t dst, bit_t a, bit_t b,
           TFHEServerParams_t = default_server_params);
void _or(bit_t dst, bit_t a, bit_t b,
         TFHEServerParams_t = default_server_params);
void _oryn(bit_t dst, bit_t a, bit_t b,
           TFHEServerParams_t = default_server_params);
void _orny(bit_t dst, bit_t a, bit_t b,
           TFHEServerParams_t = default_server_params);
void _nor(bit_t dst, bit_t a, bit_t b,
          TFHEServerParams_t = default_server_params);
void _xor(bit_t dst, bit_t a, bit_t b,
          TFHEServerParams_t = default_server_params);
void _xnor(bit_t dst, bit_t a, bit_t b,
           TFHEServerParams_t = default_server_params);

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b,
          TFHEServerParams_t = default_server_params);
void _copy(bit_t dst, bit_t src, TFHEServerParams_t = default_server_params);

void add(bitspan_t result, bitspan_t a, bitspan_t b,
         TFHEServerParams_t = default_server_params);
void mult(bitspan_t result, bitspan_t a, bitspan_t b,
          TFHEServerParams_t = default_server_params);
#endif // FHETOOLS_TFHE_H