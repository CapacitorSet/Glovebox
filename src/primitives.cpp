#include <omp.h>
#include "types.h"
#include "tfhe.h"

// todo remove
#define NUM_THREADS 4

void zero(bitspan_t src, TFHEClientParams_t p) {
	for (auto bit : src)
		constant(bit, 0, p);
}

void zero(bitspan_t src, TFHEServerParams_t p) {
	for (auto bit : src)
		constant(bit, 0, p);
}

// No bounds checking is done!
void _memcpy(bitspan_t dst, bitspan_t src, size_t size, TFHEServerParams_t p) {
	for (int i = 0; i < size; i++)
		_copy(dst[i], src[i], p);
}

void _copy(bitspan_t dst, bitspan_t src, TFHEServerParams_t p) {
	assert(dst.size() == src.size());
	_memcpy(dst, src, dst.size(), p);
}

// Warning: unlike with the full adder, A and B must not be the same variables as out
void _half_adder(bit_t out, bit_t carry_out, bit_t A, bit_t B, TFHEServerParams_t p) {
	_xor(out, A, B, p);
	_and(carry_out, A, B, p);
}

void _full_adder(bit_t out, bit_t carry_out, bit_t A, bit_t B, bit_t carry_in, TFHEServerParams_t p) {
	bit_t AxorB = make_bit(p);
	bit_t AxorBandCIn = make_bit(p);
	bit_t AandB = make_bit(p);

	_xor(AxorB, A, B, p);
	_and(AxorBandCIn, AxorB, carry_in, p);
	_and(AandB, A, B, p);

	// Instructions must be in this specific order to allow for cases like out == A or CIn == COut.
	_xor(out, AxorB, carry_in, p); // S = (A XOR B) XOR CIn
	// COut = ((A XOR B) AND CIn) OR (A AND B)
	_or(carry_out, AxorBandCIn, AandB, p);

	free_bitspan(AxorB);
	free_bitspan(AxorBandCIn);
	free_bitspan(AandB);
}

void add(bitspan_t result, bitspan_t a, bitspan_t b, TFHEServerParams_t p) {
	assert(result.size() == a.size());
	assert(a.size() == b.size());

	// Inputs
	bit_t CIn = make_bit(p);
	constant(CIn, 0, p);

	// Intermediate variables
	bit_t AxorB = make_bit(p);
	bit_t AxorBandCIn = make_bit(p);
	bit_t AandB = make_bit(p);

	// Output variables
	bit_t COut = make_bit(p);
	for (int i = 0; i < a.size(); i++) {
		bit_t A = a[i];
		bit_t B = b[i];
		bit_t S = result[i];

		_full_adder(S, COut, A, B, S, p);

		// The current COut will be used as CIn.
		_copy(CIn, COut, p);
	}
}

void mult(bitspan_t result, bitspan_t a, bitspan_t b, TFHEServerParams_t _p) {
	assert(a.size() == b.size());
	const size_t size = a.size();

	bitspan_t first_operand = make_bitspan(size, _p);
	_and(result[0], a[0], b[0], _p);
	for (int i = 0; i < size; i++)
		_and(first_operand[i], a[i], b[0], _p);

	bit_t carry = make_bit(_p);
	constant(carry, 0, _p);

	bit_t masked_operand_bit = make_bit(_p);

	/* The code is rather complex, but it essentially calculates the product doing B-masked shift-sums of A.
	 *
	 * Todo: write detailed documentation about this
	 * Todo: find a way to parallelize easily
	 */

	for (int j = 0; j < size - 1; j++) {
		printf("Multiplication progress: %d/%d\n", j, size - 2);
		bitspan_t masked_second_operand = make_bitspan(size, _p);
		for (int i = 0; i < size; i++)
			_and(masked_second_operand[i], a[i], b[j + 1], _p);

		_full_adder(result[0], carry, first_operand[1], masked_second_operand[0], carry, _p);
		// I think this can be refactored to use first_operand[0] too?
		for (int i = 1; i < size - 1; i++)
			_full_adder(first_operand[i], carry, first_operand[i + 1], masked_second_operand[i], carry, _p);
		_half_adder(first_operand[size - 1], carry, masked_second_operand[size - 1], carry, _p);
	}

	_memcpy(result.subspan(size - 1, size), first_operand, size, _p);

	free_bitspan(first_operand);
	free_bitspan(carry);
	free_bitspan(masked_operand_bit);

	// clean up
	/*
	for(int i = 0; i < size; i++)
		delete_gate_bootstrapping_ciphertext_array(size, p[i]);
	*/
}