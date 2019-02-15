#include <cassert>
#include <cstring>
#include <unistd.h>
#include <sstream>

#include <types/int.h>

void Int8::add(Int8 a, Int8 b) {
	bit_t carry = make_bit(p);
	::add8(carry, data, a.data, b.data, p);
}

void Int8::copy(Int8 src) {
	for (int i = 0; i < 8; i++)
		_copy(data[i], src.data[i], p);
}

int8_t Int8::toI8(TFHEClientParams_t p) {
	int8_t ret = 0;
	for (int i = 0; i < 8; i++)
		ret |= (::decrypt(data[i], p) & 1) << i;
	return ret;
}
