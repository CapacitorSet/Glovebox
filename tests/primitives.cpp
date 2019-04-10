#include "FHEContext.cpp"
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>

using PrimitivesTest = FHEContext;

TEST_F(PrimitivesTest, IncrIf) {
	::rc::detail::checkGTest([=](int16_t plaintext_num, bool plaintext_cond) {
		bit_t cond = make_bit(clientParams);
		encrypt(cond, plaintext_cond, clientParams);
		Int16 num(plaintext_num, clientParams);
		num.increment_if(cond);
		int16_t plaintext_expected = plaintext_num + int16_t(plaintext_cond);
		RC_ASSERT(num.toInt(clientParams) == plaintext_expected);
	});
}

TEST_F(PrimitivesTest, DecrIf) {
	::rc::detail::checkGTest([=](int16_t plaintext_num, bool plaintext_cond) {
		bit_t cond = make_bit(clientParams);
		encrypt(cond, plaintext_cond, clientParams);
		Int16 num(plaintext_num, clientParams);
		num.decrement_if(cond);
		int16_t plaintext_expected = plaintext_num - int16_t(plaintext_cond);
		RC_ASSERT(num.toInt(clientParams) == plaintext_expected);
	});
}

TEST_F(PrimitivesTest, IsZero) {
	::rc::detail::checkGTest([=](int16_t plaintext_num) {
		Int16 num(plaintext_num, clientParams);
		bit_t isZero = num.is_zero();
		bit_t isNonZero = num.is_nonzero();
		RC_ASSERT(decrypt(isZero, clientParams) ^
		          decrypt(isNonZero, clientParams));
		RC_ASSERT((num.toInt(clientParams) == 0) ==
		          (decrypt(isZero, clientParams)));
	});
}