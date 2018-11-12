#include "types.h"
#include "primitives.h"

float Fixed32::toFloat(TFHEClientParams_t _p) {
	uint16_t integer = 0;
	uint16_t fractional = 0;
	for (int i = 0; i < 16; i++) {
		fractional |= decrypt(data[i], _p) << i;
	}
	for (int i = 16; i < 32; i++) {
		integer |= decrypt(data[i], _p) << (i - 16);
	}
	return (float) (integer) + ((float) (fractional) / ((float) (1 << 16)));
}

Fixed32 Fixed32::plus(Fixed32 src) {
	Fixed32 out = Fixed32(p);

	bit_t carry = make_bit(p);
	constant(carry, 0, p);

	for (int i = 0; i < 32; i++)
		_full_adder(out.data[i], carry, data[i], src.data[i], carry, p);

	free_bitspan(carry);

	return out;
}

Fixed32 Fixed32::times(Fixed32 src) {
	Fixed32 out = Fixed32(p);

	auto dummy = make_bitspan(2 * 32, p);
	mult(dummy, data, src.data, p);
	_copy(out.data, dummy.subspan(16, 32), p); // Select the middle (make a Q16.16 from Q32.32)
	return out;
}