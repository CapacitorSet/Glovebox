#include "flow-control.h"

void _if(bit_t cond, maskable_function_t iftrue) {
	iftrue(cond);
}

void _if_else(bit_t cond, maskable_function_t iftrue, maskable_function_t iffalse, TFHEServerParams_t p) {
	iftrue(cond);
	bit_t inverted = make_bit(p);
	_not(inverted, cond, p);
	iffalse(inverted);
	free_bitspan(inverted);
}

maskable_function_t _m_if(bit_t cond, maskable_function_t iftrue, TFHEServerParams_t p) {
	return [=] (bit_t mask) {
		bit_t tmp = make_bit(p);
		_or(tmp, mask, cond, p);
		_if(tmp, iftrue);
		free_bitspan(tmp);
	};
}

maskable_function_t _m_if_else(bit_t cond, maskable_function_t iftrue, maskable_function_t iffalse,
                               TFHEServerParams_t p) {
	return [=] (bit_t mask) {
		bit_t tmp = make_bit(p);
		_or(tmp, mask, cond, p);
		_if_else(tmp, iftrue, iffalse, p);
		free_bitspan(tmp);
	};
}