#include "FHEContext.cpp"
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>

using PrimitivesTest = FHEContext;

TEST_F(PrimitivesTest, IncrIf) {
	::rc::detail::checkGTest([=](int16_t plaintext_num, bool plaintext_cond) {
		bit_t cond = make_bit();
		encrypt(cond, plaintext_cond);
		Int16 num(plaintext_num);
		increment_if(num.data, cond);
		int16_t plaintext_expected = plaintext_num + int16_t(plaintext_cond);
		RC_ASSERT(num.toInt() == plaintext_expected);
	});
}

TEST_F(PrimitivesTest, DecrIf) {
	::rc::detail::checkGTest([=](int16_t plaintext_num, bool plaintext_cond) {
		bit_t cond = make_bit();
		encrypt(cond, plaintext_cond);
		Int16 num(plaintext_num);
		decrement_if(num.data, cond);
		int16_t plaintext_expected = plaintext_num - int16_t(plaintext_cond);
		RC_ASSERT(num.toInt() == plaintext_expected);
	});
}

TEST_F(PrimitivesTest, IsZero) {
	::rc::detail::checkGTest([=](int16_t plaintext_num) {
		Int16 num(plaintext_num);
		bit_t isZero = is_zero(num.data);
		bit_t isNonZero = is_nonzero(num.data);
		RC_ASSERT(decrypt(isZero) ^ decrypt(isNonZero));
		RC_ASSERT((num.toInt() == 0) == (decrypt(isZero)));
	});
}

TEST_F(PrimitivesTest, Equals) {
	::rc::detail::checkGTest([=](int16_t plaintext_a, int16_t plaintext_b) {
		Int16 a(plaintext_a), b(plaintext_b);

		bit_t _is_equal = equals(a.data, b.data);
		bool is_equal = decrypt(_is_equal);

		RC_ASSERT((plaintext_a == plaintext_b) == is_equal);
	});
}

TEST_F(PrimitivesTest, Compare) {
	::rc::detail::checkGTest([=](int16_t plaintext_a, int16_t plaintext_b) {
		Int16 a(plaintext_a), b(plaintext_b);

		gb::bitvec<3> comparison = compare(a.data, b.data);
		bool is_lt = decrypt(comparison[0]);
		bool is_equal = decrypt(comparison[1]);
		bool is_gt = decrypt(comparison[2]);

		RC_ASSERT((plaintext_a < plaintext_b) == is_lt);
		RC_ASSERT((plaintext_a == plaintext_b) == is_equal);
		RC_ASSERT((plaintext_a > plaintext_b) == is_gt);
	});
}

TEST_F(PrimitivesTest, Abs) {
	::rc::detail::checkGTest([=](int16_t plaintext_a) {
		Int16 a(plaintext_a);

		abs(a.data);

		RC_ASSERT(abs(plaintext_a) == a.toInt());
	});
}

TEST_F(PrimitivesTest, MemImportExport) {
	::rc::detail::checkGTest([=](std::vector<char> _src) {
		char *src = _src.data();
		size_t len = _src.size();

		bitvec_t tmp = make_bitvec(len*8);
		memimport(tmp, src, len);
		char *out = new char[len];
		memset(out, 0, len);
		memexport(out, tmp, len);

		RC_ASSERT(memcmp(out, src, len) == 0);
		delete[] out;
	});
}

TEST_F(PrimitivesTest, SignExtendIncr) {
	::rc::detail::checkGTest([=](int8_t plaintext_src) {
		Int8 src(plaintext_src);
		Int16 dst;

		sign_extend(dst.data, 16, src.data, 8);

		RC_ASSERT(dst.toInt() == src.toInt());
		RC_ASSERT(dst.toInt() == plaintext_src);
	});
}
TEST_F(PrimitivesTest, SignExtendDecr) {
	::rc::detail::checkGTest([=](int16_t plaintext_src) {
		Int16 src(plaintext_src);
		Int8 dst;

		sign_extend(dst.data, 8, src.data, 16);

		RC_ASSERT(dst.toInt() == int8_t(src.toInt()));
		RC_ASSERT(dst.toInt() == int8_t(plaintext_src));
	});
}