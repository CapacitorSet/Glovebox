#ifndef FHE_CPU_TFHE_H
#define FHE_CPU_TFHE_H

#include <cstdint>
#include <cstdio>
#include <gsl_span_custom.h>

#if PLAINTEXT
typedef gsl::span<bool> bitspan_t;
typedef gsl::span<bool, 1> bit_t;
typedef struct {} TFHEServerParams_t;
typedef struct {
	operator TFHEServerParams_t() { return {}; }
} TFHEClientParams_t;
#else
#include <tfhe/tfhe.h>

using bit_t = gsl::span<LweSample, 1>;
using bitspan_t = gsl::span<LweSample>;

typedef struct {
	const TFheGateBootstrappingParameterSet *params;
	/* Todo: remove the const qualifier, deal with the issue that TFHEServerParams_t can be created either from a file
	 * (resulting in `TFHEGateBoostrappingCloudKeySet*`) or from TFHEClientParams_t (resulting in `const TFHEGate...Set*`).
	 */
	const TFheGateBootstrappingCloudKeySet *bk;
} TFHEServerParams_t;
typedef struct {
	TFheGateBootstrappingSecretKeySet* key;
	const TFheGateBootstrappingParameterSet* params;
	operator TFHEServerParams_t() {
		return TFHEServerParams_t{
			key->cloud.params,
			&(key->cloud)
		};
	}
} TFHEClientParams_t;
#endif

TFHEClientParams_t makeTFHEClientParams(FILE *secret_key);
TFHEServerParams_t makeTFHEServerParams(FILE *cloud_key);
void freeTFHEServerParams(TFHEServerParams_t p);
void freeTFHEClientParams(TFHEClientParams_t p);

extern TFHEServerParams_t default_server_params;
extern TFHEClientParams_t default_client_params;
bit_t make_bit(TFHEServerParams_t p = default_server_params);
bit_t make_bit(TFHEClientParams_t p);
bit_t make_server_bit(TFHEServerParams_t p = default_server_params);
bit_t make_client_bit(TFHEClientParams_t p = default_client_params);
bitspan_t make_bitspan(int N, TFHEClientParams_t p);
bitspan_t make_bitspan(int N, TFHEServerParams_t p = default_server_params);
bitspan_t make_client_bitspan(int N, TFHEClientParams_t p = default_client_params);
bitspan_t make_server_bitspan(int N, TFHEServerParams_t p = default_server_params);

void free_bitspan(bitspan_t item);

int decrypt(bit_t dst,
#if PLAINTEXT
		// Enable one to write just `decrypt(bit)`, since the client params aren't used anyway
		TFHEClientParams_t p = {}
#else
		TFHEClientParams_t p = default_client_params
#endif
);

void constant(bit_t dst, bool src, TFHEServerParams_t p = default_server_params);
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
#endif // FHE_CPU_TFHE_H