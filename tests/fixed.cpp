#include "FHEContext.cpp"
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>

using Q4_4 = Fixed<4, 4>;
using Q4_4Test = FHEContext;

// This is the maximum absolute error on a "measurement" (i.e. rounding to Q4.4)
constexpr double half_precision = 1.0 / 32.0;

// Rescale to within the range of a Q4.4. Use int16_t to introduce noise in
// lower bits, that should be ignored but rounded correctly. RapidCheck doesn't
// yet support doubles in ranges:
// https://github.com/emil-e/rapidcheck/issues/134
double rescale(int16_t a) {
	return double(a) / double(4096);
}

TEST_F(Q4_4Test, Decrypt) {
	::rc::detail::checkGTest([=](int16_t _plaintext_num) {
		double plaintext_num = rescale(_plaintext_num);
		auto a = Q4_4(plaintext_num);
		double absolute_error = fabs(a.toDouble() - plaintext_num);
		RC_ASSERT(absolute_error <= half_precision);
	});
}

TEST_F(Q4_4Test, Serialization) {
	::rc::detail::checkGTest([=](int16_t _plaintext_num) {
		double plaintext_num = rescale(_plaintext_num);
		auto in = Q4_4(plaintext_num);
		Q4_4 out(in.serialize());
		double absolute_error = fabs(out.toDouble() - plaintext_num);
		RC_ASSERT(absolute_error <= half_precision);
	});
}

TEST_F(Q4_4Test, SignExtend) {
	::rc::detail::checkGTest([=](int16_t _plaintext_num) {
		double plaintext_num = rescale(_plaintext_num);
		auto num = Q4_4(plaintext_num);
		auto upscaled = fixed_extend<8, 4, 4>(num);
		double absolute_error = fabs(upscaled.toDouble() - plaintext_num);
		RC_ASSERT(absolute_error <= half_precision);
	});
}

// Error propagation: absolute error of the sum = sum of absolute errors of
// addends
TEST_F(Q4_4Test, Sum) {
	::rc::detail::checkGTest([=](int16_t _plaintext_a, int16_t _plaintext_b) {
		double plaintext_a = rescale(_plaintext_a);
		double plaintext_b = rescale(_plaintext_b);
		auto a = Q4_4(plaintext_a);
		auto b = Q4_4(plaintext_b);
		auto overflow = make_bit();
		Q4_4 sum;
		sum.add(overflow, a, b);
		double plaintext_sum = plaintext_a + plaintext_b;
		bool plaintext_overflow = plaintext_sum > Q4_4::max;
		bool plaintext_underflow = plaintext_sum < Q4_4::min;
		RC_ASSERT((plaintext_overflow || plaintext_underflow) == decrypt(overflow));
		if (!plaintext_overflow && !plaintext_underflow) {
			double absolute_error = fabs(plaintext_sum - sum.toDouble());
			RC_ASSERT(absolute_error <= 2 * half_precision);
		}
	});
}

TEST_F(Q4_4Test, Mul) {
	::rc::detail::checkGTest([=](int16_t _plaintext_a, int16_t _plaintext_b) {
		double plaintext_a = rescale(_plaintext_a);
		double plaintext_b = rescale(_plaintext_b);
		// Zero-values mess up error calculations
		RC_PRE(plaintext_a != 0.0);
		RC_PRE(plaintext_b != 0.0);
		auto a = Q4_4(plaintext_a);
		auto b = Q4_4(plaintext_b);
		auto overflow = make_bit();
		Q4_4 result;
		result.mul(overflow, a, b);
		double plaintext_result = plaintext_a * plaintext_b;
		double fixed_result = a.toDouble() * b.toDouble();
		// Overflow calculations must necessarily be run on the rounded,
		// fixed-point version
		bool fixed_overflow = fixed_result > Q4_4::max || fixed_result < Q4_4::min;
		RC_ASSERT(fixed_overflow == decrypt(overflow));
		if (!fixed_overflow) {
			/* Let the relative error for X be e_X, and the absolute error be
			 * delta_X (difference between plaintext "true" value and calculated
			 * value). Then error propagation theory shows that delta_result =
			 * |result|*(|delta_a/a| + |delta_b/b| + |delta_a/a*delta_b/b|).
			 * Proof: https://math.stackexchange.com/a/3160015/53589
			 */
			double e_a = fabs(half_precision / plaintext_a);
			double e_b = fabs(half_precision / plaintext_b);
			double theoretical_absolute_error = fabs(plaintext_result) * (e_a + e_b + e_a * e_b);
			RC_ASSERT(fabs(result.toDouble() - plaintext_result) <= theoretical_absolute_error);
		}
	});
}