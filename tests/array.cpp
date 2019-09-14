#include "FHEContext.cpp"
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>
#include <types/array.h>
#include <types/fixed.h>

using ArrayTest = FHEContext;

// Read and write from an array with plaintext indices
TEST_F(ArrayTest, ReadWritePlaintext) {
	::rc::detail::checkGTest([=](int8_t plaintext_num, uint8_t _index) {
		uint8_t index = _index % 8;
		// No need for init, uninitialized regions won't be read
		Array<Int8, 8> arr(false);
		Int8 elem(plaintext_num);
		arr.put(elem, index);

		Int8 out;
		arr.get(out, index);
		RC_ASSERT(out.toInt() == plaintext_num);

		bit_t mask = make_bit();
		// Any value that will never compare equal to plaintext_num will work.
		out = plaintext_num + 1;
		mask = false;
		_if(mask, arr.m_get(out, index));
		RC_ASSERT(out.toInt() != plaintext_num);

		mask = true;
		_if(mask, arr.m_get(out, index));
		RC_ASSERT(out.toInt() == plaintext_num);
	});
}

// Read and write from an array with encrypted indices
TEST_F(ArrayTest, ReadWriteEncrypted) {
	::rc::detail::checkGTest([=](int8_t plaintext_num, uint8_t plaintext_index) {
		Int8 index(plaintext_index % 8);
		Array<Int8, 8> arr;
		Int8 elem(plaintext_num);
		arr.put(elem, index);

		Int8 out;
		arr.get(out, index);
		RC_ASSERT(out.toInt() == plaintext_num);
	});
}

TEST_F(ArrayTest, MaskedReadWriteEncrypted) {
	::rc::detail::checkGTest([=](int8_t plaintext_num, uint8_t plaintext_index) {
		Int8 index(plaintext_index % 8);
		Array<Int8, 8> arr;
		bit_t mask = make_bit();
		Int8 elem(plaintext_num);

		// Put something that doesn't compare equal to plaintext_num
		Int8 different_elem(plaintext_num + 1);
		arr.put(different_elem, index);

		mask = false;
		_if(mask, arr.m_put(elem, index));
		Int8 out;
		arr.get(out, index);
		RC_ASSERT(out.toInt() != plaintext_num);

		mask = true;
		_if(mask, arr.m_put(elem, index));
		arr.get(out, index);
		RC_ASSERT(out.toInt() == plaintext_num);
	});
}

TEST_F(ArrayTest, Serialization) {
	::rc::detail::checkGTest([=](int8_t plaintext_num) {
		Array<Int8, 1> input_arr;
		Int8 elem(plaintext_num);
		input_arr.put(elem, 0);

		auto tmp = input_arr.serialize();

		Array<Int8, 1> output_arr(tmp);
		Int8 out;
		output_arr.get(out, 0);
		RC_ASSERT(out.toInt() == plaintext_num);
	});
}