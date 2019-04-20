#include "FHEContext.cpp"
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>

using Int8Test = FHEContext;

TEST_F(Int8Test, Decrypt) {
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		auto a = Int8(plaintext_num, params);
		RC_ASSERT(a.toInt(params) == plaintext_num);
	});
}

TEST_F(Int8Test, Serialization) {
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		auto in = Int8(plaintext_num, params);
		auto tmp = in.serialize();
		auto out = Int8(tmp, params);
		RC_ASSERT(out.toInt(params) == plaintext_num);
	});
}

TEST_F(Int8Test, Sum) {
	::rc::detail::checkGTest([=](int8_t plaintext_a, int8_t plaintext_b) {
		auto a = Int8(plaintext_a, params);
		auto b = Int8(plaintext_b, params);
		auto overflow = make_bit(params);
		auto sum = Int8(params);
		sum.add(overflow, a, b);
		int16_t plaintext_sum = plaintext_a + plaintext_b;
		bool plaintext_overflow =
		    plaintext_sum > std::numeric_limits<int8_t>::max();
		bool plaintext_underflow =
		    plaintext_sum < std::numeric_limits<int8_t>::min();
		RC_ASSERT(sum.toInt(params) == int8_t(plaintext_a + plaintext_b));
		RC_ASSERT((plaintext_overflow || plaintext_underflow) ==
		          decrypt(overflow, params));
	});
}

TEST_F(Int8Test, Mul) {
	::rc::detail::checkGTest([=](int8_t plaintext_a, int8_t plaintext_b) {
		auto a = Int8(plaintext_a, params);
		auto b = Int8(plaintext_b, params);
		auto sum = Int8(params);
		sum.mul(a, b);
		RC_ASSERT(sum.toInt(params) == int8_t(plaintext_a * plaintext_b));
	});
}

TEST_F(Int8Test, Div) {
	::rc::detail::checkGTest([=](int8_t plaintext_a, int8_t plaintext_b) {
		RC_PRE(plaintext_b != 0);
		auto a = Int8(plaintext_a, params);
		auto b = Int8(plaintext_b, params);
		auto sum = Int8(params);
		sum.div(a, b);
		RC_ASSERT(sum.toInt(params) == int8_t(plaintext_a / plaintext_b));
	});
}

/*******************/

using Int16Test = FHEContext;

TEST_F(Int16Test, Decrypt) {
	::rc::detail::checkGTest([=](int16_t plaintext_num) {
		auto a = Int16(plaintext_num, params);
		RC_ASSERT(a.toInt(params) == plaintext_num);
	});
}

TEST_F(Int16Test, Serialization) {
	::rc::detail::checkGTest([=](int16_t plaintext_num) {
		auto in = Int16(plaintext_num, params);
		auto tmp = in.serialize();
		auto out = Int16(tmp, params);
		RC_ASSERT(out.toInt(params) == plaintext_num);
	});
}

TEST_F(Int16Test, Sum) {
	::rc::detail::checkGTest([=](int16_t plaintext_a, int16_t plaintext_b) {
		auto a = Int16(plaintext_a, params);
		auto b = Int16(plaintext_b, params);
		auto overflow = make_bit(params);
		auto sum = Int16(params);
		sum.add(overflow, a, b);
		int32_t plaintext_sum = plaintext_a + plaintext_b;
		bool plaintext_overflow =
		    plaintext_sum > std::numeric_limits<int16_t>::max();
		bool plaintext_underflow =
		    plaintext_sum < std::numeric_limits<int16_t>::min();
		RC_ASSERT((plaintext_overflow || plaintext_underflow) ==
		          decrypt(overflow, params));
		RC_ASSERT(sum.toInt(params) == int16_t(plaintext_a + plaintext_b));
	});
}

TEST_F(Int16Test, Mul) {
	::rc::detail::checkGTest([=](int16_t plaintext_a, int16_t plaintext_b) {
		auto a = Int16(plaintext_a, params);
		auto b = Int16(plaintext_b, params);
		auto sum = Int16(params);
		sum.mul(a, b);
		RC_ASSERT(sum.toInt(params) == int16_t(plaintext_a * plaintext_b));
	});
}

TEST_F(Int16Test, Div) {
	::rc::detail::checkGTest([=](int16_t plaintext_a, int16_t plaintext_b) {
		RC_PRE(plaintext_b != 0);
		auto a = Int16(plaintext_a, params);
		auto b = Int16(plaintext_b, params);
		auto sum = Int16(params);
		sum.div(a, b);
		RC_ASSERT(sum.toInt(params) == int16_t(plaintext_a / plaintext_b));
	});
}