#include <cassert>
#include <cstring>
#include <sstream>
#include <unistd.h>

#include <types/int.h>

void Int8::add(bit_t overflow, Int8 a, Int8 b) {
	::add8(data, overflow, a.data, b.data);
}

void Int8::add(Int8 a, Int8 b) {
	bit_t overflow = make_bit();
	this->add(overflow, a, b);
}

void Int8::mul(bit_t overflow, Int8 a, Int8 b, uint8_t truncate_from) {
	auto tmp = make_bitspan<16>();
	::mul8(tmp, a.data, b.data);
	this->round(overflow, tmp, truncate_from);
}

void Int8::mul(Int8 a, Int8 b, uint8_t truncate_from) {
	bit_t over_or_underflow = make_bit();
	this->mul(over_or_underflow, a, b, truncate_from);
}

void Int8::div(Int8 a, Int8 b) {
	::div8(data, a.data, b.data);
}

void Int8::round(bit_t overflow, const bitspan_t &src, uint8_t truncate_from) {
	round_helper(overflow, src, truncate_from);
}

void Int16::add(bit_t overflow, Int16 a, Int16 b) {
	::add16(data, overflow, a.data, b.data);
}

void Int16::add(Int16 a, Int16 b) {
	bit_t overflow = make_bit();
	this->add(overflow, a, b);
}

void Int16::mul(bit_t overflow, Int16 a, Int16 b, uint8_t truncate_from) {
	auto tmp = make_bitspan<32>();
	::mul16(tmp, a.data, b.data);
	this->round(overflow, tmp, truncate_from);
}

void Int16::mul(Int16 a, Int16 b, uint8_t truncate_from) {
	bit_t overflow = make_bit();
	this->mul(overflow, a, b, truncate_from);
}

void Int16::div(Int16 a, Int16 b) {
	::div16(data, a.data, b.data);
}

void Int16::round(bit_t overflow, const bitspan_t &src, uint8_t truncate_from) {
	round_helper(overflow, src, truncate_from);
}
