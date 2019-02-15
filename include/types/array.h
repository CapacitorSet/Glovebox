#ifndef FHETOOLS_ARRAY_H
#define FHETOOLS_ARRAY_H

#include <cassert>
#include <tfhe.h>
#include <flow_control.h>
#include "type_ids.h"
#include "int.h"

template <class T> class Array {
protected:
	uint16_t wordSize;
	bitspan_t data;
	// Todo: figure out how to use `span<bitspan_t> words` to encode this
	// information
	uint64_t length;

	/*
	static void getN_thBit(bits_t ret, uint8_t N, uint8_t wordsize,
	                       bits_t address, uint8_t bitsInAddress,
	                       bits_t staticOffset, size_t dynamicOffset,
	                       TFHEServerParams_t p = default_server_params);
	                       */

public:
	static const int typeID = ARRAY_TYPE_ID;
	Array(uint64_t _length, uint16_t _wordSize, TFHEServerParams_t _p = default_server_params)
			: wordSize(_wordSize), length(_length), p(_p) {
		data = make_bitspan(_length * _wordSize, p);
	}

	void put(T src, Varint address) {
		// Todo: check that there are enough address bits
		assert(src.size() == this->wordSize);
		bit_t mask = make_bit(p);
		constant(mask, 1, p);

		putBits(src.data, address.data, 0, mask);
	}

	maskable_function_t m_put(T src, Varint address) {
		// Todo: check that there are enough address bits
		assert(src.size() == this->wordSize);
		return [=] (bit_t mask) -> void {
			putBits(src.data, address.data, 0, mask);
		};
	}

	void getp(T dst, uint64_t address) {
		assert(address < this->length);
		dst._fromBytes(
				this->data.subspan(address * this->wordSize, this->wordSize));
	}

	maskable_function_t m_getp(T dst, uint64_t address) {
		assert(address < this->length);
		return dst._m_fromBytes(
				this->data.subspan(address * this->wordSize, this->wordSize));
	}

	void get(T dst, ClientInt address) {
		// Todo: check that there are enough address bits
		assert(dst.size() == this->wordSize);
		bit_t mask = make_bit(p);
		constant(mask, 1, p);

		getBits(dst.data, address.data, 0, mask);
	}

	maskable_function_t m_get(T dst, ClientInt address) {
		// Todo: check that there are enough address bits
		assert(dst.size() == this->wordSize);
		return [=] (bit_t mask) -> void {
			getBits(dst.data, address.data, 0, mask);
		};
	}

	bit_t equals(Array arr) {
		bit_t ret = make_bit(p);
		if (arr.data.size() != data.size()) {
			// constant(ret, 0, p);
			return ret;
		}
		constant(ret, 1, p);
		bit_t equal = make_bit(p);
		for (int i = 0; i < data.size(); i++) {
			_xnor(equal, data[i], arr.data[i], p);
			_and(ret, ret, equal, p);
		}
		free_bitspan(equal);
		return ret;
	}

private:
	TFHEServerParams_t p;

	void putBits(bitspan_t src, bitspan_t address, size_t dynamicOffset,
	             bit_t mask) {
		// Writes out of bounds are a no-op. This is necessary for arrays to
		// work with sizes other than powers of two. Bound checks should be done
		// at the caller.
		if (dynamicOffset >= this->length) {
			// printf("%zu out of bounds.\n", dynamicOffset);
			return;
		}
		if (address.size() == 1) {
			// printf("Put: %zu out of %li\n", this->wordSize * dynamicOffset + N, this->length * wordSize);
			size_t offset = wordSize * dynamicOffset;
			bit_t lowerMask = make_bit(p);
			_andyn(lowerMask, mask, address[0], p);
			for (int i = 0; i < wordSize; i++)
				_mux(data[offset + i], lowerMask, src[i], data[offset + i], p);
			free_bitspan(lowerMask);

			offset += wordSize;
			bit_t upperMask = make_bit(p);
			_and(upperMask, mask, address[0], p);
			for (int i = 0; i < wordSize; i++)
				_mux(data[offset + i], upperMask, src[i], data[offset + i], p);
			free_bitspan(upperMask);
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
		bit_t lowerMask = make_bit(p);
		_andyn(lowerMask, mask, address.last(), p);
		putBits(src, address.subspan(0, address.size() - 1), dynamicOffset,
		        lowerMask);
		free_bitspan(lowerMask);

		bit_t upperMask = make_bit(p);
		_and(upperMask, mask, address.last(), p);
		putBits(src, address.subspan(0, address.size() - 1),
		        dynamicOffset + (1 << (address.size() - 1)), upperMask);
		free_bitspan(upperMask);
	}

	void getBits(bitspan_t dst, bitspan_t address, size_t dynamicOffset,
	             bit_t mask) {
		// Reads out of bounds are a no-op. This is necessary for arrays to
		// work with sizes other than powers of two. Bound checks should be done
		// at the caller.
		if (dynamicOffset >= this->length) {
			// printf("%zu out of bounds.\n", dynamicOffset);
			return;
		}
		if (address.size() == 1) {
			// printf("Put: %zu out of %li\n", this->wordSize * dynamicOffset + N, this->length * wordSize);
			size_t offset = wordSize * dynamicOffset;
			bit_t lowerMask = make_bit(p);
			_andyn(lowerMask, mask, address[0], p);
			for (int i = 0; i < wordSize; i++)
				_mux(dst[i], lowerMask, data[offset + i], dst[i], p);
			free_bitspan(lowerMask);

			offset += wordSize;
			bit_t upperMask = make_bit(p);
			_and(upperMask, mask, address[0], p);
			for (int i = 0; i < wordSize; i++)
				_mux(dst[i], upperMask, data[offset + i], dst[i], p);
			free_bitspan(upperMask);
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
		bit_t lowerMask = make_bit(p);
		_andyn(lowerMask, mask, address.last(), p);
		putBits(dst, address.subspan(0, address.size() - 1), dynamicOffset,
		        lowerMask);
		free_bitspan(lowerMask);

		bit_t upperMask = make_bit(p);
		_and(upperMask, mask, address.last(), p);
		putBits(dst, address.subspan(0, address.size() - 1),
		        dynamicOffset + (1 << (address.size() - 1)), upperMask);
		free_bitspan(upperMask);
	}
};

template <typename T> class ClientArray : public Array<T> {
public:
	ClientArray(uint64_t _length, uint16_t _wordSize, TFHEClientParams_t _p = default_client_params)
			: Array<T>(_length, _wordSize, _p), p(_p) {}

	// Copies n bytes from src to the given address (with the address given in
	// bytes).
	void putp(char *src, uint64_t address, size_t n) {
		assert((address * 8 + n) < this->length * this->wordSize);
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < 8; j++) {
				char bit = (src[i] >> j) & 1;
				constant(&this->data[(address + i) * 8 + j], bit, p);
			}
		}
	}

	void putp(T src, uint64_t address) {
		src._writeTo(
				this->data.subspan(address * this->wordSize, this->wordSize));
	}

	/*
	// Reads one word at the given address (given in words) and puts it into
	// dst. Note that the pointers are copied, so changes made to dst will be
	// reflected in the array.
	void peek(bits_t dst, uint64_t address) {
	    assert(address < this->length);
	    memcpy(dst, &this->data[address], this->wordSize);
	}
	*/

	// Decrypts one word at the given address (given in words) and puts it into
	// dst.
	void getp(char *dst, uint64_t address) {
		assert(address < this->length);
		char byte = 0;
		char bytepos = 0;
		size_t dstpos = 0;
		for (int i = 0; i < this->wordSize; i++) {
			char bit = decrypt(this->data[address * this->wordSize + i], p);
			byte |= bit << bytepos++;
			if (bytepos == 8) {
				bytepos = 0;
				dst[dstpos++] = byte;
				byte = 0;
			}
		}
	}

private:
	TFHEClientParams_t p;
};

#endif //FHETOOLS_ARRAY_H
