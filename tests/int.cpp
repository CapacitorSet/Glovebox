#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>
#include "FHEContext.cpp"

using Ints = FHEContext;

TEST_F(Ints, DecryptI8) {
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		auto a = Int8(plaintext_num, serverParams);
		RC_ASSERT(a.toI8(clientParams) == plaintext_num);
	});
}