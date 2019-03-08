#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>
#include "FHEContext.cpp"

using Int8Test = FHEContext;

TEST_F(Int8Test, Decrypt) {
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		auto a = Int8(plaintext_num, clientParams);
		RC_ASSERT(a.toInt(clientParams) == plaintext_num);
	});
}

TEST_F(Int8Test, Serialization) {
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		auto in = Int8(plaintext_num, clientParams);
		auto tmp = in.exportToString();
		auto out = Int8(tmp, serverParams);
		RC_ASSERT(out.toInt(clientParams) == plaintext_num);
	});
}

TEST_F(Int8Test, Sum) {
	::rc::detail::checkGTest([=](int8_t plaintext_a, int8_t plaintext_b) {
		auto a = Int8(plaintext_a, clientParams);
		auto b = Int8(plaintext_b, clientParams);
		auto overflow = make_bit(serverParams);
		auto sum = Int8(serverParams);
		sum.add(overflow, a, b);
		int16_t plaintext_sum = plaintext_a + plaintext_b;
		bool plaintext_overflow = plaintext_sum > std::numeric_limits<int8_t>::max();
		bool plaintext_underflow = plaintext_sum < std::numeric_limits<int8_t>::min();
		RC_ASSERT(sum.toInt(clientParams) == int8_t(plaintext_a + plaintext_b));
		RC_ASSERT((plaintext_overflow || plaintext_underflow) == decrypt(overflow, clientParams));
	});
}

TEST_F(Int8Test, Mul) {
	::rc::detail::checkGTest([=](int8_t plaintext_a, int8_t plaintext_b) {
		auto a = Int8(plaintext_a, clientParams);
		auto b = Int8(plaintext_b, clientParams);
		auto sum = Int8(serverParams);
		sum.mul(a, b);
		RC_ASSERT(sum.toInt(clientParams) == int8_t(plaintext_a * plaintext_b));
	});
}

/*******************/

using Int16Test = FHEContext;

TEST_F(Int16Test, Decrypt) {
	::rc::detail::checkGTest([=](int16_t plaintext_num) {
		auto a = Int16(plaintext_num, clientParams);
		RC_ASSERT(a.toInt(clientParams) == plaintext_num);
	});
}

TEST_F(Int16Test, Serialization) {
	::rc::detail::checkGTest([=](int16_t plaintext_num) {
		auto in = Int16(plaintext_num, clientParams);
		auto tmp = in.exportToString();
		auto out = Int16(tmp, serverParams);
		RC_ASSERT(out.toInt(clientParams) == plaintext_num);
	});
}

TEST_F(Int16Test, Sum) {
	::rc::detail::checkGTest([=](int16_t plaintext_a, int16_t plaintext_b) {
		auto a = Int16(plaintext_a, clientParams);
		auto b = Int16(plaintext_b, clientParams);
		auto overflow = make_bit(serverParams);
		auto sum = Int16(serverParams);
		sum.add(overflow, a, b);
		int32_t plaintext_sum = plaintext_a + plaintext_b;
		bool plaintext_overflow = plaintext_sum > std::numeric_limits<int16_t>::max();
		bool plaintext_underflow = plaintext_sum < std::numeric_limits<int16_t>::min();
		RC_ASSERT((plaintext_overflow || plaintext_underflow) == decrypt(overflow, clientParams));
		RC_ASSERT(sum.toInt(clientParams) == int16_t(plaintext_a + plaintext_b));
	});
}

TEST_F(Int16Test, Mul) {
	::rc::detail::checkGTest([=](int16_t plaintext_a, int16_t plaintext_b) {
		auto a = Int16(plaintext_a, clientParams);
		auto b = Int16(plaintext_b, clientParams);
		auto sum = Int16(serverParams);
		sum.mul(a, b);
		RC_ASSERT(sum.toInt(clientParams) == int16_t(plaintext_a * plaintext_b));
	});
}