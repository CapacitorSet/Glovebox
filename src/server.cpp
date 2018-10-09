#include <cassert>
#include <unistd.h>
#include <cstring>
#include <istream>
#include <types.h>

ServerInt::ServerInt(uint8_t _size, bool _isSigned, TFHEServerParams_t _p) : p(_p) {
	size = _size;
	isSigned = _isSigned;
	data = make_bits(_size, p);
}

void ServerInt::add(ServerInt a, ServerInt b) {
  assert(size == a.size);
  assert(isSigned == a.isSigned);
  assert(a.size == b.size);
  assert(a.isSigned == b.isSigned);

  // Inputs
  bits_t A, B;
  bits_t CIn = make_bits(1, p);
  constant(CIn, 0, p);

  // Intermediate variables
  bits_t AxorB = make_bits(1, p);
  bits_t AxorBandCIn = make_bits(1, p);
  bits_t AandB = make_bits(1, p);

  // Output variables
  bits_t COut = make_bits(1, p);
  bits_t S;
  for (int i = 0; i < size; i++) {
    A = &a.data[i];
    B = &b.data[i];
    S = &data[i]; // Write to self

    _xor(AxorB, A, B, p);
    _and(AxorBandCIn, AxorB, CIn, p);
    _and(AandB, A, B, p);

    _or(COut, AxorBandCIn, AandB, p); // COut = ((A XOR B) AND CIn) OR (A AND B)
    _xor(S, AxorB, CIn, p);           // S = (A XOR B) XOR CIn

    // The current COut will be used as CIn.
    _copy(CIn, COut, p);
    /* For some reason this hack doesn't work.
    free_bits(CIn);
    CIn = COut;
    */
  }
}

void ServerInt::copy(ServerInt src) {
  assert(size == src.size);
  assert(isSigned == src.isSigned);
  for (int i = 0; i < size; i++) {
    _copy(&data[i], &src.data[i], p);
  }
}

void ServerInt::writeU8(uint8_t val) {
	assert(size == 8);
	assert(!isSigned);
	for (int i = 0; i < 8; i++) {
		constant(&data[i], (val >> i) & 1, p);
	}
}

void ServerInt::exportToFile(FILE *out) {
	for (int i = 0; i < size; i++)
		export_gate_bootstrapping_ciphertext_toFile(out, &data[i], p.params);
}