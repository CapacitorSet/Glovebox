#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>
#include "FHEContext.cpp"

using Q4_4 = Fixed<4, 4>;
using Q4_4Test = FHEContext;

TEST_F(Q4_4Test, Decrypt) {
	::rc::detail::checkGTest([=](double plaintext_num) {
		auto a = Q4_4(plaintext_num, clientParams);
		ASSERT_DOUBLE_EQ(a.toDouble(clientParams), plaintext_num);
		// RC_ASSERT(a.toDouble(clientParams) == plaintext_num);
	});
}

TEST_F(Q4_4Test, Serialization) {
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		auto in = Int8(plaintext_num, clientParams);
		auto tmp = in.exportToString();
		auto tmp_cstr = tmp.c_str();
		auto out = Int8(tmp_cstr, tmp.length(), serverParams);
		RC_ASSERT(out.toInt(clientParams) == plaintext_num);
	});
}

TEST_F(Q4_4Test, Sum) {
	::rc::detail::checkGTest([=](int8_t plaintext_a, int8_t plaintext_b) {
		auto a = Int8(plaintext_a, clientParams);
		auto b = Int8(plaintext_b, clientParams);
		auto sum = Int8(serverParams);
		sum.add(a, b);
		RC_ASSERT(sum.toInt(clientParams) == int8_t(plaintext_a + plaintext_b));
	});
}