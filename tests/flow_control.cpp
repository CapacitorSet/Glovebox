#include "FHEContext.cpp"
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>

using FlowControlTest = FHEContext;

TEST_F(FlowControlTest, If) {
	::rc::detail::checkGTest([=](bool plaintext_cond) {
		bit_t should_match_cond = make_bit(clientParams);
		encrypt(should_match_cond, false, clientParams);
		bit_t cond = make_bit(serverParams);
		constant(cond, plaintext_cond, serverParams);
		_if(cond,
		    [&](bit_t mask) { _copy(should_match_cond, mask, serverParams); });
		RC_ASSERT(decrypt(should_match_cond, clientParams) == plaintext_cond);
	});
}

TEST_F(FlowControlTest, IfElse) {
	::rc::detail::checkGTest([=](bool plaintext_cond) {
		bit_t touched_if_true = make_bit(clientParams);
		bit_t touched_if_false = make_bit(clientParams);
		encrypt(touched_if_true, false, clientParams);
		encrypt(touched_if_false, false, clientParams);
		bit_t cond = make_bit(serverParams);
		constant(cond, plaintext_cond, serverParams);
		_if_else(
		    cond,
		    [&](bit_t mask) { _copy(touched_if_true, mask, serverParams); },
		    [&](bit_t mask) { _copy(touched_if_false, mask, serverParams); },
		    serverParams);
		if (plaintext_cond) {
			RC_ASSERT(decrypt(touched_if_true, clientParams) == true);
			RC_ASSERT(decrypt(touched_if_false, clientParams) == false);
		} else {
			RC_ASSERT(decrypt(touched_if_true, clientParams) == false);
			RC_ASSERT(decrypt(touched_if_false, clientParams) == true);
		}
	});
}

TEST_F(FlowControlTest, DISABLED_While) {}

TEST_F(FlowControlTest, Times) {
	::rc::detail::checkGTest([=](int8_t plaintext_n, uint8_t max) {
		// Increment `tmp` for `n` times.
		// If n <= max then we expect tmp = n.
		// Otherwise we expect tmp = max, overflow == false.
		Int8 n(plaintext_n, clientParams);
		Int8 tmp(0, clientParams);
		bit_t overflow = times(
		    n, max, [&](bit_t mask) { tmp.increment_if(mask); }, serverParams);
		if (plaintext_n < 0) {
			// Expect that nothing was done.
			RC_ASSERT(tmp.toInt(clientParams) == 0);
			return;
		}
		if (plaintext_n <= max) {
			RC_ASSERT(decrypt(overflow, clientParams) == false);
			RC_ASSERT(tmp.toInt(clientParams) == plaintext_n);
		} else {
			RC_ASSERT(decrypt(overflow, clientParams) == true);
			RC_ASSERT(tmp.toInt(clientParams) == max);
		}
	});
}