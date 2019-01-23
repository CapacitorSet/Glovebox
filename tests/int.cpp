#include <types.h>
#include "gtest/gtest.h"
#include "FHEContext.cpp"

TEST_F(FHEContext, DecryptClientU8) {
	// This is not a real encryption, but the name is kept for consistency
	uint8_t val = 123;
	auto a = ClientInt::newU8(val, clientParams);
	ASSERT_EQ(a->toU8(clientParams), val);
}

TEST_F(FHEContext, DecryptServerU8) {
	uint8_t val = 123;
	auto a = Int::newU8(val, serverParams);
	ASSERT_EQ(a->toU8(clientParams), val);
}

// Tests that a server uint of arbitrary, small size decrypts correctly
TEST_F(FHEContext, DecryptUInt) {
	// This is not a real encryption, but the name is kept for consistency
	uint8_t size = 18;
	uint16_t val = 0x02FF;
	auto nbytes = size / 2 + 1;
	auto a = new Int(size, false, serverParams);
	char *dst = static_cast<char *>(calloc(nbytes, 1));
	a->decrypt(dst, clientParams);
	printf("%x %x\n", dst[0], dst[1]);
	ASSERT_EQ(memcmp(&val, dst, nbytes), 0);
	// auto a = ???
	// a->decrypt()
	// ASSERT_EQ(..., val);
}

// Tests that a server int of arbitrary, small size decrypts correctly
TEST_F(FHEContext, DISABLED_DecryptInt) {
	uint8_t size = 18;
	uint8_t val = 123;
	// auto a = ???
	// a->decrypt()
	// ASSERT_EQ(..., val);
}