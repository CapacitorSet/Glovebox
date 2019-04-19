#include <tfhe.h>

#warning You are compiling a plaintext binary.

TFHEClientParams_t makeTFHEClientParams(FILE *) { return TFHEClientParams_t{}; }

TFHEServerParams_t makeTFHEServerParams(FILE *) { return TFHEServerParams_t{}; }

TFHEServerParams_t makeTFHEServerParams(TFHEClientParams_t) {
	return TFHEServerParams_t{};
}

void freeTFHEServerParams(TFHEServerParams_t) {}
void freeTFHEClientParams(TFHEClientParams_t) {}

int decrypt(bit_t dst, TFHEClientParams_t) { return *dst.data(); }

bit_t make_bit(weak_params_t) {
	auto ptr = std::shared_ptr<bool>(reinterpret_cast<bool *>(malloc(1)));
	return gsl::span<bool, 1>(ptr, 1);
}

bitspan_t make_bitspan(int N, weak_params_t) {
	auto ptr = std::shared_ptr<bool>(reinterpret_cast<bool *>(malloc(N)));
	return gsl::span<bool>(ptr, N);
}

void encrypt(bit_t dst, bool src, TFHEClientParams_t) { *dst.data() = src; }

void _unsafe_constant(bit_t dst, bool src, weak_params_t) {
	*dst.data() = src;
};

void _not(bit_t dst, bit_t src, weak_params_t) { *dst.data() = !*src.data(); }

#define BINARY_OPERATOR(LibName, CppExpr)                                      \
	void _##LibName(bit_t dst, const bit_t a, const bit_t b, weak_params_t) {  \
		auto A = *a.data();                                                    \
		auto B = *b.data();                                                    \
		*dst.data() = (CppExpr);                                               \
	}

BINARY_OPERATOR(and, A &&B)
BINARY_OPERATOR(andyn, A && !B)
BINARY_OPERATOR(andny, !A && B)
BINARY_OPERATOR(nand, !(A && B))
BINARY_OPERATOR(or, (A || B))
BINARY_OPERATOR(oryn, A || !B)
BINARY_OPERATOR(orny, !A || B)
BINARY_OPERATOR(nor, !(A || B))
BINARY_OPERATOR(xor, A ^ B)
BINARY_OPERATOR(xnor, !(A ^ B))

void _mux(bit_t dst, bit_t cond, bit_t a, bit_t b, weak_params_t) {
	*dst.data() = *cond.data() ? *a.data() : *b.data();
}

void _copy(bit_t dst, bit_t src, weak_params_t) { *dst.data() = *src.data(); }
