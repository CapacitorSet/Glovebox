#include <utility>

#ifndef FHE_TOOLS_TYPES_H
#define FHE_TOOLS_TYPES_H

#include "tfhe.h"
#include "primitives.h"
#include "flow-control.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <glob.h>

// The enum is not actually used, it just ensures that type IDs do not overlap
enum {
	UNINITIALIZED_TYPE_ID = 0,
	INT_TYPE_ID,
	ARRAY_TYPE_ID,
};

typedef struct {
	char *ptr;
	size_t len;
} ptr_with_length_t;

class Int {
	template <class T> friend class Array;

  protected:
	Int(uint8_t _size, bool _isSigned, TFHEServerParams_t _p,
	    bool initialize = true)
	    : isSigned(_isSigned), p(_p) {
		if (initialize)
			data = make_bitspan(_size, p);
	}

	bool isSigned;
	bitspan_t data;

  public:
	// virtual void writeU8(uint8_t) = 0;

	ptr_with_length_t exportToChar();
	void exportToFile(FILE *out);
	void print(TFHEClientParams_t p);
	void sprint(char *out, TFHEClientParams_t p);
	void free() {
		// it only actually works if the free is commented out.
		// todo: figure out wtf is up
		// free_bitspan(data);
	}

	// todo: rename
	void _writeTo(bitspan_t dst) { _copy(dst, data, p); }
	void _fromBytes(bitspan_t dst) { _copy(data, dst, p); }
	maskable_function_t _m_fromBytes(bit_t mask, bitspan_t dst) {
		return [=] (bit_t mask) -> void {
			for (int i = 0; i < data.size(); i++)
				_mux(data[i], mask, dst[i], data[i], p);
		};
	}

	static const int typeID = INT_TYPE_ID;
	const int size() const { return data.size(); }
	const bool &getSigned() const { return isSigned; }

	Int(char *packet, size_t pktsize, TFHEServerParams_t _p);
	static Int *newU8(TFHEServerParams_t p) { return new Int(8, false, p); }
	static Int *newU8(uint8_t n, TFHEServerParams_t p) {
		auto ret = Int::newU8(p);
		ret->writeU8(n);
		return ret;
	}
	void writeU8(uint8_t);

	void add(Int, Int);
	void mult(Int, Int);
	void copy(Int);

	bit_t isZero() {
		bit_t ret = make_bit(p);
		constant(ret, 1, p);
		for (const auto bit : data)
			_andyn(ret, ret, bit, p);
		return ret;
	}

	bit_t isNonZero() {
		bit_t ret = make_bit(p);
		constant(ret, 0, p);
		for (const auto bit : data)
			_or(ret, ret, bit, p);
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

  private:
	TFHEClientParams_t p;
};

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
	                       TFHEServerParams_t p);
	                       */

  public:
	static const int typeID = ARRAY_TYPE_ID;
	Array(uint64_t _length, uint16_t _wordSize, TFHEServerParams_t _p)
	    : wordSize(_wordSize), length(_length), p(_p) {
		data = make_bitspan(_length * _wordSize, p);
	}

	void put(T src, ClientInt address) {
		// Todo: check that there are enough address bits
		assert(src.size() == this->wordSize);
		bit_t mask = make_bit(p);
		constant(mask, 1, p);

		putBits(src.data, address.data, 0, mask);
	}

	maskable_function_t m_put(T src, ClientInt address) {
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
	ClientArray(uint64_t _length, uint16_t _wordSize, TFHEClientParams_t _p)
	    : p(_p), Array<T>(_length, _wordSize, makeTFHEServerParams(_p)) {}

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

// A string of *fixed* length.
class String : protected Array<Int> {
public:
	String(uint16_t len, TFHEServerParams_t p) : Array<Int>(len, 8, p) {}
	String(char *src, TFHEServerParams_t p) : Array<Int>(strlen(src), 8, p) {
		for (size_t i = 0; i < length; i++)
			for (int j = 0; j < 8; j++)
				constant(data[i * 8 + j], src[i], p);
	}

	void toCStr(char *dst, TFHEClientParams_t _p) {
		for (size_t i = 0; i < length; i++) {
			char out = 0;
			for (int j = 0; j < 8; j++)
				out |= decrypt(data[i * 8 + j], _p) << j;
			dst[i] = out;
		}
		dst[length] = 0;
	}

	bit_t equals(String dst) {
		return Array::equals(dst);
	}
};

class ClientString: public String {
public:
	ClientString(uint16_t len, TFHEClientParams_t p) : String(len, makeTFHEServerParams(p)) {}
	ClientString(char *src, TFHEClientParams_t p) : String(src, makeTFHEServerParams(p)) {
		size_t len = strlen(src);
		for (size_t i = 0; i < length; i++)
			for (int j = 0; j < 8; j++)
				constant(data[i * 8 + j], (src[i] >> j) & 1, p);
	}
};

// 16 bit integer part + 16 bit fractional
class Fixed32 {
public:
	Fixed32(float src, TFHEServerParams_t _p) : Fixed32(_p) {
		int32_t tmp = (int32_t) src; // Extract integer part
		src -= tmp;
		tmp <<= 16;
		tmp += (int32_t) (src * (1 << 16));

		for (int i = 0; i < 32; i++)
			constant(data[i], (tmp >> i) & 1, p);
	}

	float toFloat(TFHEClientParams_t _p);

	Fixed32 plus(Fixed32 src);
	Fixed32 times(Fixed32 src);

	void free() {
		free_bitspan(data);
	}

protected:
	explicit Fixed32(TFHEServerParams_t _p) : p(_p) {
		data = make_bitspan(32, p);
	}

	bitspan_t data;
	TFHEServerParams_t p;
};

class ClientFixed32 : public Fixed32 {
public:
	ClientFixed32(float src, TFHEClientParams_t _p) : Fixed32(makeTFHEServerParams(_p)) {
		data = make_bitspan(32, p);

		int32_t tmp = (int32_t) src; // Extract integer part
		src -= tmp;
		tmp <<= 16;
		tmp += (int32_t) (src * (1 << 16));

		for (int i = 0; i < 32; i++)
			constant(data[i], (tmp >> i) & 1, p);
	}
};

class KnownPolynomial {
public:
	KnownPolynomial(std::vector<Fixed32> _factors, TFHEServerParams_t _p) : factors(std::move(_factors)), p(_p) {
	}
	KnownPolynomial(std::vector<float> _factors, TFHEServerParams_t _p) : p(_p) {
		for (auto _factor : _factors) {
			factors.push_back(Fixed32(_factor, _p));
		}
	}

	Fixed32 evaluate(Fixed32 x);

private:
	std::vector<Fixed32> factors;

	TFHEServerParams_t p;
};

#endif // FHE_TOOLS_TYPES_H
