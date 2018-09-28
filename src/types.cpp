#include "types.h"
#include "debug.h"
#include <cassert>
#include <types.h>
#include <unistd.h>
#include <cstring>
#include <istream>
#include <glob.h>

FILE* charptr_to_file(char* src, size_t len) {
	char filename[] = "/tmp/fhetools_charptr_to_file.dat";
	FILE *wptr = fopen(filename, "wb");
	assert(wptr);
	fwrite(src, len, 1, wptr);
	fclose(wptr);

	FILE *rptr = fopen(filename, "rb");
	assert(rptr);
	return rptr;
}

ptr_with_length_t Int::exportToChar() {
	char header[2];
	memcpy(header, &isSigned, 1);
	memcpy(header + 1, &size, 1);
	printf("Header: %#04x %#04x\n", header[0], header[1]);
	char filename[] = "/tmp/fhetools_exportToChar.dat";
	FILE *wptr = fopen(filename, "wb");
	assert(wptr);
	exportToFile(wptr);
	fclose(wptr);

	FILE *rptr = fopen(filename, "rb");
	assert(rptr);
	// https://stackoverflow.com/a/238609
	fseek(rptr, 0, SEEK_END);
	long _size = ftell(rptr) + sizeof(header);
	assert(_size <= SIZE_MAX); // So it can be cast to size_t safely
	size_t size = static_cast<size_t>(_size);
	fseek(rptr, 0, SEEK_SET);
	char* out = static_cast<char *>(malloc(size));
	memcpy(out, header, sizeof(header));
	fread(out + sizeof(header), 1, size, rptr);
	fclose(rptr);
	// unlink(filename);
	printf("Out: %#04x %#04x\n", out[0], out[1]);
	return ptr_with_length_t{out, size};
}

ClientInt::ClientInt(uint8_t _size, bool _isSigned, TFHEClientParams_t _p) : p(_p) {
	size = _size;
	isSigned = _isSigned;
	data = make_bits(_size, p);
}

void ClientInt::writeU8(uint8_t val) {
	assert(size == 8);
	assert(!isSigned);
	for (int i = 0; i < 8; i++) {
		constant(&data[i], (val >> i) & 1, p);
	}
}

// todo: document that it doesn't export a header
void ClientInt::exportToFile(FILE *out) {
	for (int i = 0; i < size; i++)
		export_gate_bootstrapping_ciphertext_toFile(out, &data[i], p.params);
}

ServerInt::ServerInt(char *packet, size_t pktsize, TFHEServerParams_t p) {
	memcpy(&isSigned, packet, 1);
	memcpy(&size, packet + 1, 1);
	// Skip header
	packet += 2; pktsize -= 2;
	FILE* f = charptr_to_file(packet, pktsize);
	if (f == nullptr) {
		printf("Value of errno: %d\n", errno);
		perror("Error printed by perror");
		printf("Error opening file: %s\n", strerror( errno ));
		exit(-1);
	}
	assert(f);
	data = make_bits(size, p);
	for (int i = 0; i < size; i++)
		import_gate_bootstrapping_ciphertext_fromFile(f, &data[i], p.params);
}

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
	export_gate_bootstrapping_ciphertext_toFile(out, data, p.params);
}

#if DEBUG
void ServerInt::print() {
  for (int i = 8; i-- > 0;) {
    printf("%d", decrypt(&data[i], p));
  }
  putchar('\n');
}
#endif

Array::Array(uint64_t _length, uint16_t _wordSize, TFHEServerParams_t _p) {
  length = _length;
  wordSize = _wordSize;
  data = make_bits(_length * _wordSize, _p);
}

void Array::getN_thInt(ServerInt ret, const bits_t address, uint8_t bitsInAddress) {
	for (int i = 0; i < ret.size; i++) {
		Array::getN_thBit(&ret.data[i], i, ret.size, address, bitsInAddress, this->data, 0, p);
	}
}

void Array::putN_thInt(ServerInt src, const bits_t address, uint8_t bitsInAddress) {
	bits_t mask = make_bits(1, p);
	constant(mask, 1, p);
	for (int i = 0; i < src.size; i++) {
		Array::putN_thBit(&src.data[i], i, src.size, address, bitsInAddress, this->data, 0, mask, p);
	}
}

/* Puts into ret the Nth bit of a variable at a given `address` into a memory `staticOffset`
 */
void Array::getN_thBit(bits_t ret, uint8_t N, uint8_t wordsize, const bits_t address, uint8_t bitsInAddress,
                       const bits_t staticOffset, size_t dynamicOffset, TFHEServerParams_t p) {
  if (bitsInAddress == 1) {
    _mux(ret, &address[0],
		&staticOffset[wordsize * (dynamicOffset + 1) + N],
		&staticOffset[wordsize * (dynamicOffset    ) + N],
		 p);
    return;
  }
#if TRIVIAL_getNth_bit
  // Avoids branching, doing simple recursion instead
	int bit = decrypt(&address[bitsInAddress - 1]);
	if (bit) {
		getN_thBit(ret, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset + (1 << (bitsInAddress - 1)));
	} else {
		getN_thBit(ret, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset);
	}
#else
	/*
  // Would branching result in an offset so high it will read out of bounds?
  /* This can naturally happen if an instruction is reading one word ahead (eg.
   * to read the argument). getN_thbit will scan the entire memory, and when
   * it scans the last word, the branch "read one word ahead" will overflow.
   * As a consequence of this, if the machine intentionally reads out of bounds
   * it will read zeros rather than segfaulting - but you would never want to
   * read past the memory size, right?
  int willBranchOverflow = (N + 8 * (1 + dynamicOffset + (1 << (bitsInAddress - 1)))) >= MEMSIZE;
  if (willBranchOverflow) {
    // If yes, force the result to stay in bounds: return the lower branch only.
    getN_thBit(ret, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset, p);
    return;
  }
	*/
  bits_t a = make_bits(1, p);
  getN_thBit(a, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset + (1 << (bitsInAddress - 1)), p);
  bits_t b = make_bits(1, p);
  getN_thBit(b, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset, p);
  _mux(ret, &address[bitsInAddress - 1], a, b, p);
  free_bits(a);
  free_bits(b);
#endif
}

/* Puts into an array staticOffset, at the `address`, at the Nth bit in the word, a bit `src`
 */
void Array::putN_thBit(bits_t src, uint8_t N, uint8_t wordsize, const bits_t address, uint8_t bitsInAddress,
                       const bits_t staticOffset, size_t dynamicOffset, bits_t mask, TFHEServerParams_t p) {
	assert(N < wordsize);
	if (bitsInAddress == 1) {
		bits_t bit;
		bit = &staticOffset[wordsize * (dynamicOffset    ) + N];
		bits_t lowerMask = make_bits(1, p);
		_andyn(lowerMask, mask, &address[0], p);
		_mux(bit, lowerMask, src, bit, p);
		free_bits(lowerMask);

		bit = &staticOffset[wordsize * (dynamicOffset + 1) + N];
		bits_t upperMask = make_bits(1, p);
		_and(upperMask, mask, &address[0], p);
		_mux(bit, upperMask, src, bit, p);
		free_bits(lowerMask);
		return;
	}
	/*
	// Would branching result in an offset so high it will read out of bounds?
	/* This can naturally happen if an instruction is reading one word ahead (eg.
	 * to read the argument). getN_thbit will scan the entire memory, and when
	 * it scans the last word, the branch "read one word ahead" will overflow.
	 * As a consequence of this, if the machine intentionally reads out of bounds
	 * it will read zeros rather than segfaulting - but you would never want to
	 * read past the memory size, right?
	int willBranchOverflow = (N + 8 * (1 + dynamicOffset + (1 << (bitsInAddress - 1)))) >= MEMSIZE;
	if (willBranchOverflow) {
		// If yes, force the result to stay in bounds: return the lower branch only.
		putN_thBit(src, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset, mask, p);
		return;
	}
	 */
	bits_t upperMask = make_bits(1, p);
	_and(upperMask, mask, &address[bitsInAddress - 1], p);
#if TRIVIAL_putNth_bit
	if (decrypt(upperMask))
#endif
	putN_thBit(src, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset + (1 << (bitsInAddress - 1)), upperMask, p);
	free_bits(upperMask);

	bits_t lowerMask = make_bits(1, p);
	_andyn(lowerMask, mask, &address[bitsInAddress - 1], p);
#if TRIVIAL_putNth_bit
	if (decrypt(lowerMask))
#endif
	putN_thBit(src, N, wordsize, address, bitsInAddress - 1, staticOffset, dynamicOffset, lowerMask, p);
	free_bits(lowerMask);
}
