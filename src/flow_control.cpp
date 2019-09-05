#include "flow_control.h"

void _if(bit_t cond, maskable_function_t iftrue) {
	iftrue(cond);
}

void _if_else(bit_t cond, maskable_function_t iftrue, maskable_function_t iffalse) {
	iftrue(cond);
	bit_t inverted = make_bit();
	_not(inverted, cond);
	iffalse(inverted);
}

maskable_function_t _m_if(bit_t cond, maskable_function_t iftrue) {
	return [=](bit_t mask) {
		bit_t tmp = make_bit();
		_or(tmp, mask, cond);
		_if(tmp, iftrue);
	};
}

maskable_function_t _m_if_else(bit_t cond, maskable_function_t iftrue,
                               maskable_function_t iffalse) {
	return [=](bit_t mask) {
		bit_t tmp = make_bit();
		_or(tmp, mask, cond);
		_if_else(tmp, iftrue, iffalse);
	};
}

bit_t _while(std::function<bit_t(void)> condition, uint64_t max, maskable_function_t body) {
	bit_t mask = make_bit();
	constant(mask, true);
	for (uint64_t i = 0; i < max; i++) {
		// This line is required if the condition is temporarily false but then
		// is true again. That's weird behaviour, but it might happen.
		mask &= condition();
		body(mask);
	}
	// Check if the mask is still high after leaving the loop.
	// If so, max was not sufficient; "overflow".
	mask &= condition();
	return mask;
}

bit_t times(const Int8 src, uint8_t max, maskable_function_t body) {
	Int8 tmp;
	_copy(tmp.data, src.data);
	bit_t mask = make_bit();
	_andyn(mask, is_nonzero(src.data), is_negative(src.data));
	for (uint8_t i = 0; i < max; i++) {
		body(mask);
		decrement_if(tmp.data, mask);
		mask &= is_nonzero(tmp.data);
	}
	// The loop should not be enabled by the last step.
	return mask;
}
