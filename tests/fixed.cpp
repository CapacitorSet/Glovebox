#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>
#include "FHEContext.cpp"

using Q4_4 = Fixed<4, 4>;
using Q4_4Test = FHEContext;

// Round to within the resolution of a Q4.4, which is 2^-4
double quantize(double a) {
	return round(a * 16.f) / 16.f;
}
// Rescale to within the range of a Q4.4. Use int16_t to introduce noise in lower bits, that
// should be ignored but rounded correctly.
// RapidCheck doesn't yet support doubles in ranges: https://github.com/emil-e/rapidcheck/issues/134
double rescale(int16_t a) {
	return double(a) / double(4096);
}

TEST_F(Q4_4Test, Decrypt) {
	::rc::detail::checkGTest([=](int16_t _plaintext_num) {
		double plaintext_num = rescale(_plaintext_num);
		auto a = Q4_4(plaintext_num, clientParams);
		RC_ASSERT(quantize(a.toDouble(clientParams)) == quantize(plaintext_num));
	});
}

TEST_F(Q4_4Test, Serialization) {
	::rc::detail::checkGTest([=](int16_t _plaintext_num) {
		double plaintext_num = rescale(_plaintext_num);
		auto in = Q4_4(plaintext_num, clientParams);
		auto tmp = in.exportToString();
		auto out = Q4_4(tmp, serverParams);
		RC_ASSERT(quantize(out.toDouble(clientParams)) == quantize(plaintext_num));
	});
}

TEST_F(Q4_4Test, Sum) {
	::rc::detail::checkGTest([=](int16_t _plaintext_a, int16_t _plaintext_b) {
		double plaintext_a = rescale(_plaintext_a);
		double plaintext_b = rescale(_plaintext_b);
		auto a = Q4_4(plaintext_a, clientParams);
		auto b = Q4_4(plaintext_b, clientParams);
		auto overflow = make_bit(serverParams);
		auto sum = Q4_4(serverParams);
		sum.add(overflow, a, b);
		// Quantization errors can add up, and I'm too lazy to implement the actual
		// check (which is abs(quantize(sum) - quantize(plaintext_sum)) <= 2*resolution)
		double plaintext_sum = quantize(plaintext_a) + quantize(plaintext_b);
		bool plaintext_overflow = plaintext_sum > Q4_4::max;
		bool plaintext_underflow = plaintext_sum < Q4_4::min;
		RC_ASSERT((plaintext_overflow || plaintext_underflow) == decrypt(overflow, clientParams));
		if (!plaintext_overflow && !plaintext_underflow)
			RC_ASSERT(quantize(sum.toDouble(clientParams)) == quantize(plaintext_sum));
	});
}

TEST_F(Q4_4Test, UpscaleToQ8_4) {
	::rc::detail::checkGTest([=](int16_t _plaintext_num) {
		double plaintext_num = rescale(_plaintext_num);
		auto num = Q4_4(plaintext_num, clientParams);
		auto upscaled = fixed_extend<8, 4, 4>(num, clientParams);
		RC_ASSERT(quantize(upscaled.toDouble(clientParams)) == quantize(plaintext_num));
	});
}