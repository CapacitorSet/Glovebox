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

void Int8::mul(bit_t overflow, Int8 a, Int8 b, uint8_t truncate_from) {
	auto tmp = make_bitspan<16>(p);
	bit_t dummy = make_bit(p);
	::mul8(tmp, dummy, a.data, b.data, p);
	_copy(data, tmp.subspan(truncate_from, 8), p);
	// todo: compute overflow (= if any of the truncated bits are set to 0)
	::constant(overflow, false, p);
}

void Int8::mul(Int8 a, Int8 b, uint8_t truncate_from) {
	bit_t over_or_underflow = make_bit(p);
	this->mul(over_or_underflow, a, b, truncate_from);
}

void Int8::copy(Int8 src) {
	for (int i = 0; i < 8; i++)
		_copy(data[i], src.data[i], p);
}
