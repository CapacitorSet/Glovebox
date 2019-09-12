#include "FHEContext.cpp"
#include <flow_control.h>
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>

using FlowControlTest = FHEContext;

TEST_F(FlowControlTest, If) {
	::rc::detail::checkGTest([=](bool plaintext_cond) {
		bit_t should_match_cond = make_bit();
		encrypt(should_match_cond, false);
		bit_t cond = make_bit();
		encrypt(cond, plaintext_cond);
		_if(cond, [&](bit_t mask) { _copy(should_match_cond, mask); });
		RC_ASSERT(decrypt(should_match_cond) == plaintext_cond);
	});
}

TEST_F(FlowControlTest, IfElse) {
	::rc::detail::checkGTest([=](bool plaintext_cond) {
		bit_t touched_if_true = make_bit();
		bit_t touched_if_false = make_bit();
		encrypt(touched_if_true, false);
		encrypt(touched_if_false, false);
		bit_t cond = make_bit();
		encrypt(cond, plaintext_cond);
		_if_else(
		    cond, [&](bit_t mask) { _copy(touched_if_true, mask); },
		    [&](bit_t mask) { _copy(touched_if_false, mask); });
		if (plaintext_cond) {
			RC_ASSERT(decrypt(touched_if_true) == true);
			RC_ASSERT(decrypt(touched_if_false) == false);
		} else {
			RC_ASSERT(decrypt(touched_if_true) == false);
			RC_ASSERT(decrypt(touched_if_false) == true);
		}
	});
}

TEST_F(FlowControlTest, DISABLED_While) {}

TEST_F(FlowControlTest, Times) {
	::rc::detail::checkGTest([=](int8_t plaintext_n, uint8_t max) {
		// Increment `tmp` for `n` times.
		// If n <= max then we expect tmp = n.
		// Otherwise we expect tmp = max, overflow == false.
		Int8 n(plaintext_n);
		Int8 tmp(0);
		bit_t overflow = times(n, max, [&](bit_t mask) { increment_if(tmp.data, mask); });
		if (plaintext_n < 0) {
			// Expect that nothing was done.
			RC_ASSERT(tmp.toInt() == 0);
			return;
		}
		if (plaintext_n <= max) {
			RC_ASSERT(decrypt(overflow) == false);
			RC_ASSERT(tmp.toInt() == plaintext_n);
		} else {
			RC_ASSERT(decrypt(overflow) == true);
			RC_ASSERT(tmp.toInt() == max);
		}
	});
}