#include "gtest/gtest.h"
#include "FHEContext.cpp"

// Ugly way to iterate on (false, true)
#define ITERATE_BIT(name) for (char name = false; !name; name ^= 1)

#define TEST_OP(FHEOperand, CppExpression) TEST_F(FHEContext, FHEOperand) { \
	ITERATE_BIT(A) ITERATE_BIT(B) { \
        char out = CppExpression; \
        bit_t fhe_A = make_bit(serverParams); \
        constant(fhe_A, A, serverParams); \
        bit_t fhe_B = make_bit(serverParams); \
        constant(fhe_B, B, serverParams); \
        bit_t fhe_out = make_bit(serverParams); \
        _ ## FHEOperand(fhe_out, fhe_A, fhe_B, serverParams); \
        ASSERT_EQ(decrypt(fhe_out, clientParams), out); \
        free_bitspan(fhe_A); \
        free_bitspan(fhe_B); \
        free_bitspan(fhe_out); \
    } \
}

TEST_OP(and,   A && B);
TEST_OP(andny, !A && B);
TEST_OP(andyn, A && !B);
TEST_OP(or,    A || B);
TEST_OP(xor,   A ^ B);
TEST_OP(nand,  !(A && B));
TEST_OP(nor,   !(A || B));
TEST_OP(xnor,  !(A ^ B));

TEST_F(FHEContext, DecryptFromClient) {
	ITERATE_BIT(plaintext) {
		bit_t ciphertext = make_bit(clientParams);
		constant(ciphertext, plaintext, clientParams);
		ASSERT_EQ(decrypt(ciphertext, clientParams), plaintext);
	}
}

TEST_F(FHEContext, DecryptFromServer) {
	ITERATE_BIT(plaintext) {
		bit_t ciphertext = make_bit(serverParams);
		constant(ciphertext, plaintext, serverParams);
		ASSERT_EQ(decrypt(ciphertext, clientParams), plaintext);
	}
}