#ifndef FHETOOLS_TFHE_H
#define FHETOOLS_TFHE_H

#include <cstdint>
#include <gsl_span_custom.h>
#include <tfhe/tfhe.h>

#if PLAINTEXT
using unsafe_bit_t = bool;
typedef struct {} only_TFHEServerParams_t;
typedef struct {
	operator only_TFHEServerParams_t() {
		return only_TFHEServerParams_t{};
	}
} TFHEServerParams_t;
typedef struct {
	operator TFHEServerParams_t() {
		return TFHEServerParams_t{};
	}
	operator only_TFHEServerParams_t() = delete; // If you can read this you're passing client params to server-only functions.
} TFHEClientParams_t;
#else
using unsafe_bit_t = LweSample;

typedef struct {
	const TFheGateBootstrappingParameterSet *params;
	/* Todo: remove the const qualifier, deal with the issue that TFHEServerParams_t can be created either from a file
	 * (resulting in `TFHEGateBoostrappingCloudKeySet*`) or from TFHEClientParams_t (resulting in `const TFHEGate...Set*`).
	 */
	const TFheGateBootstrappingCloudKeySet *bk;
} only_TFHEServerParams_t ;
typedef struct {
	const TFheGateBootstrappingParameterSet *params;
	const TFheGateBootstrappingCloudKeySet *bk;
	operator only_TFHEServerParams_t() {
		return only_TFHEServerParams_t{params, bk};
	}
} TFHEServerParams_t ;
typedef struct {
	TFheGateBootstrappingSecretKeySet* key;
	const TFheGateBootstrappingParameterSet* params;
	operator TFHEServerParams_t() {
		return TFHEServerParams_t{
			key->cloud.params,
			&(key->cloud)
		};
	}
	operator only_TFHEServerParams_t() = delete; // If you can read this you're passing client params to server-only functions.
} TFHEClientParams_t;
#endif

typedef gsl::span<unsafe_bit_t> bitspan_t;
typedef gsl::span<unsafe_bit_t, 1> bit_t;

TFHEClientParams_t makeTFHEClientParams(FILE *secret_key);
TFHEServerParams_t makeTFHEServerParams(FILE *cloud_key);
void freeTFHEServerParams(only_TFHEServerParams_t p);
void freeTFHEClientParams(TFHEClientParams_t p);

extern TFHEServerParams_t default_server_params;
extern TFHEClientParams_t default_client_params;
bit_t make_bit(TFHEServerParams_t p = default_server_params);
bit_t make_bit(TFHEClientParams_t p);
bit_t make_server_bit(only_TFHEServerParams_t p = default_server_params);
bit_t make_client_bit(TFHEClientParams_t p = default_client_params);
bitspan_t make_bitspan(int N, TFHEClientParams_t p);
bitspan_t make_bitspan(int N, TFHEServerParams_t p = default_server_params);
bitspan_t make_client_bitspan(int N, TFHEClientParams_t p = default_client_params);
bitspan_t make_server_bitspan(int N, only_TFHEServerParams_t p = default_server_params);

void free_bitspan(bitspan_t item);

int decrypt(bit_t dst,
#if PLAINTEXT
		// Enable one to write just `decrypt(bit)`, since the client params aren't used anyway
		TFHEClientParams_t p = {}
#else
		TFHEClientParams_t p = default_client_params
#endif
);

void constant(bit_t dst, bool src, only_TFHEServerParams_t p = default_server_params);
void encrypt(bit_t dst, bool src, TFHEClientParams_t p = default_client_params);

void _not(bit_t dst, bit_t a, TFHEServerParams_t p = default_server_params);
void _and(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p = default_server_params);
void _andyn(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p = default_server_params);
void _andny(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p = default_server_params);
void _nand(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p = default_server_params);
void _or(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p = default_server_params);
void _oryn(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p = default_server_params);
void _orny(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p = default_server_params);
void _nor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p = default_server_params);
void _xor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p = default_server_params);
void _xnor(bit_t dst, bit_t a, bit_t b, TFHEServerParams_t p = default_server_params);

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b, TFHEServerParams_t p = default_server_params);
void _copy(bit_t dst, bit_t src, TFHEServerParams_t p = default_server_params);

void add(bitspan_t result, bitspan_t a, bitspan_t b, TFHEServerParams_t p = default_server_params);
void mult(bitspan_t result, bitspan_t a, bitspan_t b, TFHEServerParams_t _p = default_server_params);
#endif // FHETOOLS_TFHE_H