#include "FHEContext.cpp"
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>

using ArrayTest = FHEContext;

// Read and write from an array with plaintext indices
TEST_F(ArrayTest, ReadWritePlaintext) {
	::rc::detail::checkGTest([=](int8_t plaintext_num, uint8_t _index) {
		uint8_t index = _index % 8;
		// No need for init, uninitialized regions won't be read
		auto arr = Array<Int8, 8>(false);
		auto elem = Int8(plaintext_num);
		arr.put(elem, index);

		auto out = Int8();
		arr.get(out, index);
		RC_ASSERT(out.toInt() == plaintext_num);
	});
}

// Read and write from an array with encrypted indices
TEST_F(ArrayTest, ReadWriteEncrypted) {
	::rc::detail::checkGTest([=](int8_t plaintext_num, uint8_t index) {
		Int8 index_int = Int8(index % 8);
		auto arr = Array<Int8, 8>(true);
		auto elem = Int8(plaintext_num);
		arr.put(elem, index_int);

		auto out = Int8(0); // Initialize memory so ubsan doesn't complain
		arr.get(out, index_int);
		RC_ASSERT(out.toInt() == plaintext_num);
	});
}

TEST_F(ArrayTest, Serialization) {
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		auto input_arr = Array<Int8, 1>(true);
		auto elem = Int8(plaintext_num);
		input_arr.put(elem, 0);

		auto tmp = input_arr.serialize();

		auto output_arr = Array<Int8, 1>(tmp);
		auto out = Int8();
		output_arr.get(out, 0);
		RC_ASSERT(out.toInt() == plaintext_num);
	});
}