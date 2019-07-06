#include "FHEContext.cpp"
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>

using StringTest = FHEContext;

constexpr uint16_t TEST_SIZE = 5;

TEST_F(StringTest, Decrypt) {
	::rc::detail::checkGTest([=](std::string plaintext_str) {
		plaintext_str.resize(TEST_SIZE - 1);
		auto a = String<TEST_SIZE>(plaintext_str.c_str(), ModePicker::CLIENT);
		char str[TEST_SIZE];
		a.toCStr(str);
		RC_ASSERT(strcmp(plaintext_str.c_str(), str) == 0);
	});
}

TEST_F(StringTest, Serialization) {
	::rc::detail::checkGTest([=](std::string plaintext_str) {
		plaintext_str.resize(TEST_SIZE - 1);
		auto in = String<TEST_SIZE>(plaintext_str.c_str(), ModePicker::CLIENT);
		auto tmp = in.serialize();
		auto out = String<TEST_SIZE>(tmp);
		char str[TEST_SIZE];
		out.toCStr(str);
		RC_ASSERT(strcmp(plaintext_str.c_str(), str) == 0);
	});
}