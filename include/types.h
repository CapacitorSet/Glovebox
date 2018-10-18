#ifndef FHE_TOOLS_TYPES_H
#define FHE_TOOLS_TYPES_H

#include "tfhe.h"
#include <cassert>
#include <cstdint>
#include <glob.h>

// The enum is not actually used, it just ensures that type IDs do not overlap
enum {
	UNINITIALIZED_TYPE_ID = 0,
	INT_TYPE_ID,
	ARRAY_TYPE_ID,
};

bits_t bits_merge(bits_t, bits_t);

typedef struct {
	char *ptr;
	size_t len;
} ptr_with_length_t;

class Int {
	template <class T> friend class Array;

  public:
	// virtual void writeU8(uint8_t) = 0;

	ptr_with_length_t exportToChar();
	void exportToFile(FILE *out);
	void print(TFHEClientParams_t p);
	void sprint(char *out, TFHEClientParams_t p);
	void free() {
		// it only actually works if the free is commented out.
		// todo: figure out wtf is up

		// printf("Freeing %d items.\n", size);
		// free_LweSample_array(size, data);
	}

	void _writeTo(bits_t dst, size_t i) { _copy(dst, &this->data[i], p); }

	const int &getSize() const { return size; }
	const bool &getSigned() const { return isSigned; }

  protected:
	Int(uint8_t _size, bool _isSigned, TFHEServerParams_t _p,
	          bool initialize = true)
	    : size(_size), isSigned(_isSigned), p(_p) {
		if (initialize)
			data = make_bits(_size, p.params);
	}

	uint8_t size;
	bool isSigned;
	bits_t data;

  public:
	Int(char *packet, size_t pktsize, TFHEServerParams_t _p);
	static Int *newU8(TFHEServerParams_t p) {
		return new Int(8, false, p);
	}
	static Int *newU8(uint8_t n, TFHEServerParams_t p) {
		auto ret = Int::newU8(p);
		ret->writeU8(n);
		return ret;
	}
	void writeU8(uint8_t);

	void add(Int, Int);
	void copy(Int);

	static bits_t isZero(Int n) {
		bits_t ret = make_bits(1, n.p);
		constant(ret, 1, n.p);
		for (int i = 0; i < n.size; i++)
			_andyn(ret, ret, &n.data[i], n.p);
		return ret;
	}

	static bits_t isNonZero(Int n) {
		bits_t ret = make_bits(1, n.p);
		constant(ret, 1, n.p);
		for (int i = 0; i < n.size; i++)
			_and(ret, ret, &n.data[i], n.p);
		_not(ret, ret, n.p);
		return ret;
	}

  private:
	TFHEServerParams_t p;
};

class ClientInt : public Int {
  public:
	ClientInt(char *packet, size_t pktsize, TFHEClientParams_t _p)
	    : p(_p), Int(packet, pktsize, makeTFHEServerParams(_p)){};
	static ClientInt *newU8(TFHEClientParams_t _p) {
		return new ClientInt(8, false, _p);
	}
	static ClientInt *newU8(uint8_t n, TFHEClientParams_t p) {
		auto r = Int::newU8(makeTFHEServerParams(p));
		auto ret = ClientInt::newU8(p);
		ret->writeU8(n);
		return ret;
	}
	void writeU8(uint8_t);
	uint8_t toU8();
	ClientInt(uint8_t _size, bool _isSigned, TFHEClientParams_t _p)
	    : p(_p), Int(_size, _isSigned, makeTFHEServerParams(_p)) {}
	void operator delete(void *_ptr) {
		auto ptr = static_cast<ClientInt *>(_ptr);
		ptr->free();
	}

  private:
	TFHEClientParams_t p;
};

template <class T> class Array {
  protected:
	uint64_t length;
	uint16_t wordSize;
	bits_t data;

	static void getN_thBit(bits_t ret, uint8_t N, uint8_t wordsize,
	                       bits_t address, uint8_t bitsInAddress,
	                       bits_t staticOffset, size_t dynamicOffset,
	                       TFHEServerParams_t p);

  public:
	Array(uint64_t _length, uint16_t _wordSize, TFHEServerParams_t _p)
	    : length(_length), wordSize(_wordSize), p(_p) {
		data = make_bits(_length * _wordSize, p.params);
	}

	void put(T src, bits_t address, uint8_t bitsInAddress) {
		bits_t mask = make_bits(1, p);
		constant(mask, 1, p);
		for (int i = 0; i < src.size; i++) {
			putBit(&src.data[i], i, src.size, address, bitsInAddress,
			       this->data, 0, mask, p);
		}
	}

  private:
	TFHEServerParams_t p;

	void putBit(bits_t src, uint8_t N, uint8_t wordsize, bits_t address,
	            uint8_t bitsInAddress, bits_t staticOffset,
	            size_t dynamicOffset, bits_t mask, TFHEServerParams_t p) {
		assert(N < wordsize);
		if (bitsInAddress == 1) {
			bits_t bit;
			bit = &staticOffset[wordsize * (dynamicOffset) + N];
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
		Would branching result in an offset so high it will read out of bounds?
		This can naturally happen if an instruction is reading one word ahead
		(eg. to read the argument). getN_thbit will scan the entire memory, and
		when it scans the last word, the branch "read one word ahead" will
		overflow. As a consequence of this, if the machine intentionally reads
		out of bounds it will read zeros rather than segfaulting - but you would
		never want to read past the memory size, right?

		 int willBranchOverflow = (N + 8 * (1 + dynamicOffset + (1 <<
		(bitsInAddress
		- 1)))) >= MEMSIZE; if (willBranchOverflow) {
		    // If yes, force the result to stay in bounds: return the lower
		branch only. putN_thBit(src, N, wordsize, address, bitsInAddress - 1,
		staticOffset, dynamicOffset, mask, p); return;
		}
		 */
		bits_t upperMask = make_bits(1, p);
		_and(upperMask, mask, &address[bitsInAddress - 1], p);
		putBit(src, N, wordsize, address, bitsInAddress - 1, staticOffset,
		       dynamicOffset + (1 << (bitsInAddress - 1)), upperMask, p);
		free_bits(upperMask);

		bits_t lowerMask = make_bits(1, p);
		_andyn(lowerMask, mask, &address[bitsInAddress - 1], p);
		putBit(src, N, wordsize, address, bitsInAddress - 1, staticOffset,
		       dynamicOffset, lowerMask, p);
		free_bits(lowerMask);
	}
};

template <typename T> class ClientArray : Array<T> {
  public:
	ClientArray(uint64_t _length, uint16_t _wordSize, TFHEClientParams_t _p)
	    : p(_p), Array<T>(_length, _wordSize, makeTFHEServerParams(_p)) {}

	// Copies n bytes from src to the given address (with the address given in
	// bytes).
	void put(char *src, uint64_t address, size_t n) {
		assert((address * 8 + n) < this->length * this->wordSize);
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < 8; j++) {
				char bit = (src[i] >> j) & 1;
				printf("data[%#016x] = %d\n", (address + i) * 8 + j, bit);
				constant(&this->data[(address + i) * 8 + j], bit, p);
			}
		}
	}

	void put(T src, uint64_t address) {
		for (int i = 0; i < this->wordSize; i++) {
			src._writeTo(&this->data[address * this->wordSize + i], i);
		}
	}

	// Reads one word at the given address (given in words) and puts it into
	// dst. Note that the pointers are copied, so changes made to dst will be
	// reflected in the array.
	void peek(bits_t dst, uint64_t address) {
		assert(address < this->length);
		memcpy(dst, &this->data[address], this->wordSize);
	}

	// Decrypts one word at the given address (given in words) and puts it into
	// dst.
	void get(char *dst, uint64_t address) {
		assert(address < this->length);
		char byte;
		char bytepos = 0;
		size_t dstpos = 0;
		for (int i = 0; i < this->wordSize; i++) {
			printf("Accessing %#016x.\n", address * this->wordSize + i);
			char bit = decrypt(&this->data[address * this->wordSize + i], p);
			byte |= bit << bytepos++;
			if (bytepos == 8) {
				bytepos = 0;
				dst[dstpos++] = byte;
				byte = 0;
			}
		}
	}

	void get(T dst, uint64_t address) {
		assert(address < this->length);
		dst.fromBytes(&this->data[address], this->wordSize);
	}

  private:
	TFHEClientParams_t p;
};

#endif // FHE_TOOLS_TYPES_H
