#include <types.h>
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>
#include "FHEContext.cpp"

using Ints = FHEContext;
using IntsDeathTest = FHEContext;

TEST_F(Ints, DecryptClientI8) {
	// This is not a real encryption, but the name is kept for consistency
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		auto a = ClientInt::newI8(plaintext_num, clientParams);
		RC_ASSERT(a->toI8(clientParams) == plaintext_num);
	});
}

TEST_F(Ints, DecryptServerI8) {
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		auto a = Varint::newI8(plaintext_num, serverParams);
		RC_ASSERT(a->toI8(clientParams) == plaintext_num);
	});
}

// Tests that a server i32 decrypts correctly
TEST_F(Ints, DecryptInt) {
	::rc::detail::checkGTest([=](int32_t plaintext_num) {
		uint8_t size = 32;
		auto nbytes = 4;
		auto a = new Varint(size, serverParams);
		a->write(plaintext_num);
		char dst[nbytes];
		a->decrypt(dst, clientParams);
		RC_ASSERT(memcmp(&plaintext_num, dst, nbytes) == 0);
	});
}

TEST_F(IntsDeathTest, IntUpperBoundsCheck) {
	uint8_t size = 8;
	auto a = new Varint(size, serverParams);
	int64_t val = 1 << 16;
	ASSERT_DEATH(a->write(val), ".*");
}

TEST_F(IntsDeathTest, IntLowerBoundsCheck) {
	uint8_t size = 8;
	auto a = new Varint(size, serverParams);
	int64_t val = -(1 << 16);
	ASSERT_DEATH(a->write(val), ".*");
}