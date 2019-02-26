#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>
#include "FHEContext.cpp"

using StringTest = FHEContext;

constexpr uint16_t TEST_SIZE = 1;

TEST_F(StringTest, Decrypt) {
	::rc::detail::checkGTest([=](std::string plaintext_str) {
		plaintext_str.resize(TEST_SIZE);
		auto a = String<TEST_SIZE>(plaintext_str.c_str(), clientParams);
		char str[TEST_SIZE];
		a.toCStr(str, clientParams);
		// Compare the whole string, but at most TEST_SIZE bytes.
		auto cmp_len = std::min(plaintext_str.length(), size_t(TEST_SIZE));
		RC_ASSERT(memcmp(plaintext_str.c_str(), str, cmp_len) == 0);
	});
}

TEST_F(StringTest, Serialization) {
	::rc::detail::checkGTest([=](std::string plaintext_str) {
		plaintext_str.resize(TEST_SIZE);
		auto in = String<TEST_SIZE>(plaintext_str.c_str(), clientParams);
		auto tmp = in.exportToString();
		auto tmp_cstr = tmp.c_str();
		auto out = String<TEST_SIZE>(tmp_cstr, tmp.length(), serverParams);
		char str[TEST_SIZE];
		out.toCStr(str, clientParams);
		// Compare the whole string, but at most TEST_SIZE bytes.
		auto cmp_len = std::min(plaintext_str.length(), size_t(TEST_SIZE));
		RC_ASSERT(memcmp(plaintext_str.c_str(), str, cmp_len) == 0);
	});
}