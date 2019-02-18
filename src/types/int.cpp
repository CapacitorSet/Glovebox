#include <cassert>
#include <cstring>
#include <unistd.h>
#include <sstream>

#include <types/int.h>

void Int8::add(bit_t overflow, Int8 a, Int8 b) {
	::add8(data, overflow, a.data, b.data, p);
}

void Int8::add(Int8 a, Int8 b) {
	bit_t overflow = make_bit();
	this->add(overflow, a, b);
	free_bitspan(overflow);
}

void Int8::mul(bit_t over_or_underflow, Int8 a, Int8 b) {
	auto tmp = make_fixed_bitspan<16>(p);
	::mul8(tmp, over_or_underflow, a.data, b.data, p);
	_copy(data, tmp.first<8>(), p);
	free_bitspan(tmp);
}

void Int8::mul(Int8 a, Int8 b) {
	bit_t over_or_underflow = make_bit();
	this->mul(over_or_underflow, a, b);
	free_bitspan(over_or_underflow);
}

void Int8::copy(Int8 src) {
	for (int i = 0; i < 8; i++)
		_copy(data[i], src.data[i], p);
}
