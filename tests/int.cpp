#include <types.h>
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>
#include "FHEContext.cpp"

using Ints = FHEContext;
using IntsDeathTest = FHEContext;

TEST_F(Ints, DecryptClientU8) {
	// This is not a real encryption, but the name is kept for consistency
	::rc::detail::checkGTest([=](uint8_t plaintext_num) {
		auto a = ClientInt::newU8(plaintext_num, clientParams);
		RC_ASSERT(a->toU8(clientParams) == plaintext_num);
	});
}

TEST_F(Ints, DecryptServerU8) {
	::rc::detail::checkGTest([=](uint8_t plaintext_num) {
		auto a = Int::newU8(plaintext_num, serverParams);
		RC_ASSERT(a->toU8(clientParams) == plaintext_num);
	});
}

// Tests that a server u32 decrypts correctly
TEST_F(Ints, DecryptUInt) {
	::rc::detail::checkGTest([=](uint32_t plaintext_num) {
		uint8_t size = 32;
		auto nbytes = 4;
		auto a = new Int(size, Int::UNSIGNED, serverParams);
		a->uwrite(plaintext_num);
		char dst[nbytes];
		a->decrypt(dst, clientParams);
		RC_ASSERT(memcmp(&plaintext_num, dst, nbytes) == 0);
	});
}

// Tests that a server i32 decrypts correctly
TEST_F(Ints, DecryptInt) {
	::rc::detail::checkGTest([=](int32_t plaintext_num) {
		uint8_t size = 32;
		auto nbytes = 4;
		auto a = new Int(size, Int::SIGNED, serverParams);
		a->swrite(plaintext_num);
		char dst[nbytes];
		a->decrypt(dst, clientParams);
		RC_ASSERT(memcmp(&plaintext_num, dst, nbytes) == 0);
	});
}

TEST_F(IntsDeathTest, UIntBoundsCheck) {
	uint8_t size = 8;
	auto a = new Int(size, Int::UNSIGNED, serverParams);
	uint64_t outerVal = 1 << 16;
	ASSERT_DEATH(a->uwrite(outerVal), ".*");
}

TEST_F(IntsDeathTest, IntUpperBoundsCheck) {
	uint8_t size = 8;
	auto a = new Int(size, Int::SIGNED, serverParams);
	int64_t val = 1 << 16;
	ASSERT_DEATH(a->swrite(val), ".*");
}

TEST_F(IntsDeathTest, IntLowerBoundsCheck) {
	uint8_t size = 8;
	auto a = new Int(size, Int::SIGNED, serverParams);
	int64_t val = -(1 << 16);
	ASSERT_DEATH(a->swrite(val), ".*");
}