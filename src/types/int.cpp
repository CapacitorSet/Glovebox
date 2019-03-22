#include <cassert>
#include <cstring>
#include <unistd.h>
#include <sstream>

#include <types/int.h>

void Int8::add(bit_t overflow, Int8 a, Int8 b) {
	::add8(data, overflow, a.data, b.data, p);
}

void Int8::add(Int8 a, Int8 b) {
	bit_t overflow = make_bit(p);
	this->add(overflow, a, b);
}

void Int8::increment_if(bit_t cond) {
	auto tmp = make_bitspan<8>(p);
	::incr8_if(tmp, cond, data, p);
	_copy<8>(data, tmp, p);
}

void Int8::mul(bit_t overflow, Int8 a, Int8 b, uint8_t truncate_from) {
	auto tmp = make_bitspan<16>(p);
	::mul8(tmp, a.data, b.data, p);
	_copy(data, tmp.subspan(truncate_from, 8), p);
	// We can't just _copy: consider the case 0.011, which does not truncate
	// to 0.01 but to 0.10
	bit_t sign_bit = tmp.last();
	if (truncate_from != 0) {
		bit_t should_increment = make_bit(p);
		_xor(should_increment, sign_bit, tmp[truncate_from - 1], p);
		this->increment_if(should_increment);
	}
	// The number overflows if it is positive and has 1s past what we truncated,
	// or it is negative and has 0s past what we truncated.
	::constant(overflow, false, p);
	for (int i = truncate_from + 8; i < 16; i++) {
		bit_t is_overflowing = make_bit(p);
		_xor(is_overflowing, sign_bit, tmp[i], p);
		_or(overflow, overflow, is_overflowing, p);
	}
}

void Int8::mul(Int8 a, Int8 b, uint8_t truncate_from) {
	bit_t over_or_underflow = make_bit(p);
	this->mul(over_or_underflow, a, b, truncate_from);
}

void Int8::div(Int8 a, Int8 b) {
	::div8(data, a.data, b.data, p);
}

void Int8::copy(Int8 src) {
	for (int i = 0; i < 8; i++)
		_copy(data[i], src.data[i], p);
}

void Int16::add(bit_t overflow, Int16 a, Int16 b) {
	::add16(data, overflow, a.data, b.data, p);
}

void Int16::add(Int16 a, Int16 b) {
	bit_t overflow = make_bit(p);
	this->add(overflow, a, b);
}

void Int16::increment_if(bit_t cond) {
	auto tmp = make_bitspan<16>(p);
	::incr16_if(tmp, cond, data, p);
	_copy<16>(data, tmp, p);
}

void Int16::mul(bit_t overflow, Int16 a, Int16 b, uint16_t truncate_from) {
	auto tmp = make_bitspan<32>(p);
	::mul16(tmp, a.data, b.data, p);
	_copy(data, tmp.subspan(truncate_from, 16), p);
	// We can't just _copy: consider the case 0.011, which does not truncate
	// to 0.01 but to 0.10
	bit_t sign_bit = tmp[truncate_from - 1];
	if (truncate_from != 0)
		this->increment_if(sign_bit);

	// The number overflows if it is positive and has 1s past what we truncated,
	// or it is negative and has 0s past what we truncated.
	::constant(overflow, false, p);
	for (int i = truncate_from + 8; i < 16; i++) {
		bit_t is_overflowing = make_bit(p);
		_xor(is_overflowing, sign_bit, tmp[i], p);
		_or(overflow, overflow, tmp[i], p);
	}
}

void Int16::mul(Int16 a, Int16 b, uint16_t truncate_from) {
	bit_t over_or_underflow = make_bit(p);
	this->mul(over_or_underflow, a, b, truncate_from);
}

void Int16::copy(Int16 src) {
	for (int i = 0; i < 16; i++)
		_copy(data[i], src.data[i], p);
}

void Int16::div(Int16 a, Int16 b) {
	::div16(data, a.data, b.data, p);
}
