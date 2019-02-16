#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>
#include "FHEContext.cpp"

using Int8Test = FHEContext;

TEST_F(Int8Test, Decrypt) {
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		auto a = Int8(plaintext_num, clientParams);
		RC_ASSERT(a.toI8(clientParams) == plaintext_num);
	});
}

TEST_F(Int8Test, Serialization) {
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		auto in = Int8(plaintext_num, clientParams);
		auto tmp = in.exportToString();
		auto tmp_cstr = tmp.c_str();
		auto out = Int8(tmp_cstr, tmp.length(), serverParams);
		RC_ASSERT(out.toI8(clientParams) == plaintext_num);
	});
}

TEST_F(Int8Test, Sum) {
	::rc::detail::checkGTest([=](int8_t plaintext_a, int8_t plaintext_b) {
		auto a = Int8(plaintext_a, clientParams);
		auto b = Int8(plaintext_b, clientParams);
		auto sum = Int8(serverParams);
		sum.add(a, b);
		RC_ASSERT(sum.toI8(clientParams) == int8_t(plaintext_a + plaintext_b));
	});
}