#include <types.h>
#include "gtest/gtest.h"
#include "FHEContext.cpp"

using Ints = FHEContext;
using IntsDeathTest = FHEContext;

TEST_F(Ints, DecryptClientU8) {
	// This is not a real encryption, but the name is kept for consistency
	uint8_t val = 123;
	auto a = ClientInt::newU8(val, clientParams);
	ASSERT_EQ(a->toU8(clientParams), val);
}

TEST_F(Ints, DecryptServerU8) {
	uint8_t val = 123;
	auto a = Int::newU8(val, serverParams);
	ASSERT_EQ(a->toU8(clientParams), val);
}

// Tests that a server uint of arbitrary, small size decrypts correctly
TEST_F(Ints, DecryptUInt) {
	// This is not a real encryption, but the name is kept for consistency
	uint8_t size = 18;
	uint32_t val = 0x010203;
	auto nbytes = size / 8 + 1;
	auto a = new Int(size, false, serverParams);
	a->write(val);
	char *dst = static_cast<char *>(calloc(nbytes, 1));
	a->decrypt(dst, clientParams);
	ASSERT_FALSE(memcmp(&val, dst, nbytes));
}

// Tests that a server int of arbitrary, small size decrypts correctly
TEST_F(Ints, DISABLED_DecryptInt) {
	uint8_t size = 18;
	uint8_t val = 123;
	// auto a = ???
	// a->decrypt()
	// ASSERT_EQ(..., val);
}

TEST_F(IntsDeathTest, BoundsCheck) {
	uint8_t size = 8;
	uint64_t val = 1 << 16;
	auto a = new Int(size, false, serverParams);
	ASSERT_DEATH(a->write(val), ".*");
}