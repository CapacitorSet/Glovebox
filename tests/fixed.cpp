#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>
#include "FHEContext.cpp"

using Q4_4 = Fixed<4, 4>;
using Q4_4Test = FHEContext;

// Round to within the resolution of a Q4.4, which is 2^-4
double quantize(double a) {
	return round(a * 16.f) / 16.f;
}
// Rescale to within the range of a Q4.4.
// RapidCheck doesn't yet support doubles in ranges: https://github.com/emil-e/rapidcheck/issues/134
double rescale(int8_t a) {
	return double(a) / double(16);
}

TEST_F(Q4_4Test, Decrypt) {
	::rc::detail::checkGTest([=](int8_t _plaintext_num) {
		double plaintext_num = rescale(_plaintext_num);
		auto a = Q4_4(plaintext_num, clientParams);
		RC_ASSERT(quantize(a.toDouble(clientParams)) == quantize(plaintext_num));
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