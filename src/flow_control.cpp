#include "flow_control.h"

void _if(bit_t cond, maskable_function_t iftrue) { iftrue(cond); }

void _if_else(bit_t cond, maskable_function_t iftrue,
              maskable_function_t iffalse, TFHEServerParams_t p) {
	iftrue(cond);
	bit_t inverted = make_bit(p);
	_not(inverted, cond, p);
	iffalse(inverted);
}

maskable_function_t _m_if(bit_t cond, maskable_function_t iftrue,
                          TFHEServerParams_t p) {
	return [=](bit_t mask) {
		bit_t tmp = make_bit(p);
		_or(tmp, mask, cond, p);
		_if(tmp, iftrue);
	};
}

maskable_function_t _m_if_else(bit_t cond, maskable_function_t iftrue,
                               maskable_function_t iffalse,
                               TFHEServerParams_t p) {
	return [=](bit_t mask) {
		bit_t tmp = make_bit(p);
		_or(tmp, mask, cond, p);
		_if_else(tmp, iftrue, iffalse, p);
	};
}

bit_t _while(std::function<bit_t(void)> condition, uint64_t max,
             maskable_function_t body, TFHEServerParams_t p) {
	bit_t mask = make_bit(p);
	constant(mask, true, p);
	for (uint64_t i = 0; i < max; i++) {
		// This line is required if the condition is temporarily false but then
		// is true again. That's weird behaviour, but it might happen.
		_and(mask, mask, condition(), p);
		body(mask);
	}
	// Check if the mask is still high after leaving the loop.
	// If so, max was not sufficient; "overflow".
	_and(mask, mask, condition(), p);
	return mask;
}

bit_t times(const Int8 src, uint8_t max, maskable_function_t body,
            TFHEServerParams_t p) {
	Int8 tmp(p);
	tmp.copy(src);
	bit_t mask = make_bit(p);
	_andyn(mask, src.is_nonzero(), src.isNegative(), p);
	for (uint8_t i = 0; i < max; i++) {
		body(mask);
		tmp.decrement_if(mask);
		_and(mask, mask, tmp.is_nonzero(), p);
	}
	// The loop should not be enabled by the last step.
	return mask;
}
