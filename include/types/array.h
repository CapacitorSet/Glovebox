#ifndef FHETOOLS_ARRAY_H
#define FHETOOLS_ARRAY_H

#include <cassert>
#include <tfhe.h>
#include <flow_control.h>
#include "type_ids.h"
#include "int.h"

// Computes ceil(log_2(value)), constexpr
static constexpr uint8_t ceillog2(uint16_t value) {
	uint8_t ret = 0;
	while (value != 0) {
		value >>= 1;
		ret++;
	}
	return ret;
}

template <class T, uint16_t Length, uint16_t WordSize = T::_wordSize> class Array {
protected:
	static constexpr uint8_t AddrBits = ceillog2(Length);
private:
	using native_address_type = smallest_uint_t<AddrBits>;
	using encrypted_address_type = smallest_Int<AddrBits>;
	static_assert(AddrBits <= 8, "This size is not supported"); // See smallest_Int

	static constexpr uint32_t Bitlength = WordSize * Length;
public:
	bitspan_t data;

	/*
	static void getN_thBit(bits_t ret, uint8_t N, uint8_t wordsize,
	                       bits_t address, uint8_t bitsInAddress,
	                       bits_t staticOffset, size_t dynamicOffset,
	                       TFHEServerParams_t p = default_server_params);
	                       */

	static const int typeID = ARRAY_TYPE_ID;
	explicit Array(bool initialize_memory = true, only_TFHEServerParams_t _p = default_server_params)
			: p(unwrap_only(_p)) {
		data = make_bitspan(Bitlength, p);
		if (initialize_memory)
			zero(data, unwrap_only(_p));
	}

	explicit Array(bool initialize_memory, TFHEClientParams_t _p) : p(_p) {
		data = make_bitspan(Bitlength, p);
		if (initialize_memory)
			zero(data, _p);
	}

	Array(const std::string &packet, TFHEServerParams_t _p = default_server_params) : Array(false, _p) {
		char typeID_from_header = packet[0];
		uint16_t length_from_header;
		memcpy(&length_from_header, &packet[1], 2);
		assert(typeID_from_header == T::typeID);
		assert(length_from_header == Length);
		// Skip header
		std::stringstream ss(packet.substr(3));
		deserialize(ss, data, p);
	}

	void put(T src, encrypted_address_type address) {
		bit_t mask = make_bit(p);
		constant(mask, 1, p);

		putBits(src.data, address.data, 0, mask);
	}

	void put(T src, native_address_type address) {
		assert(address < Length);
		for (uint16_t i = 0; i < WordSize; i++)
			_copy(data[address * WordSize + i], src.data[i], p);
	};

	/*
	maskable_function_t m_put(T src, Int8 address) {
		// Todo: check that there are enough address bits
		return [=] (bit_t mask) -> void {
			putBits(src.data, address.data, 0, mask);
		};
	}
	*/

	void get(T dst, native_address_type address) {
		assert(address < Length);
		_copy(dst.data, data.subspan(address * WordSize, WordSize), p);
	}

	/*
	maskable_function_t m_get(T dst, uint64_t address) {
		assert(address < this->length);
		return dst._m_fromBytes(
				this->data.subspan(address * this->wordSize, this->wordSize));
	}
	*/

	void get(T dst, encrypted_address_type address) {
		bit_t mask = make_bit(p);
		constant(mask, 1, p);

		getBits(dst.data, address.data, 0, mask);
	}

	std::string exportToString() {
		char header[3] = {T::typeID};
		uint16_t size = Length;
		memcpy(header + 1, &size, 2);
		std::ostringstream oss;
		oss.write(header, sizeof(header));
		serialize(oss, data, p);
		return oss.str();
	}

	/*
	maskable_function_t m_get(T dst, Int8 address) {
		// Todo: check that there are enough address bits
		assert(dst.size() == this->wordSize);
		return [=] (bit_t mask) -> void {
			getBits(dst.data, address.data, 0, mask);
		};
	}
	 */

protected:
	TFHEServerParams_t p;

private:
	void putBits(bitspan_t src, bitspan_t address, size_t dynamicOffset,
	             bit_t mask) {
		// Writes out of bounds are a no-op. This is necessary for arrays to
		// work with sizes other than powers of two. Bound checks should be done
		// at the caller.
		if (dynamicOffset >= Length) {
			// printf("%zu out of bounds.\n", dynamicOffset);
			return;
		}
		if (address.size() == 1) {
			// printf("Put: %zu out of %li\n", this->wordSize * dynamicOffset + N, this->length * wordSize);
			size_t offset = WordSize * dynamicOffset;
			bit_t lowerMask = make_bit(p);
			_andyn(lowerMask, mask, address[0], p);
			for (int i = 0; i < WordSize; i++)
				_mux(data[offset + i], lowerMask, src[i], data[offset + i], p);

			offset += WordSize;
			bit_t upperMask = make_bit(p);
			_and(upperMask, mask, address[0], p);
			for (int i = 0; i < WordSize; i++)
				_mux(data[offset + i], upperMask, src[i], data[offset + i], p);
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

		bit_t upperMask = make_bit(p);
		_and(upperMask, mask, address.last(), p);
		putBits(src, address.subspan(0, address.size() - 1),
		        dynamicOffset + (1 << (address.size() - 1)), upperMask);
	}

	void getBits(bitspan_t dst, bitspan_t address, size_t dynamicOffset,
	             bit_t mask) {
		// Reads out of bounds are a no-op. This is necessary for arrays to
		// work with sizes other than powers of two. Bound checks should be done
		// at the caller.
		if (dynamicOffset >= Length) {
			// printf("%zu out of bounds.\n", dynamicOffset);
			return;
		}
		if (address.size() == 1) {
			// printf("Put: %zu out of %li\n", this->wordSize * dynamicOffset + N, this->length * wordSize);
			size_t offset = WordSize * dynamicOffset;
			bit_t lowerMask = make_bit(p);
			_andyn(lowerMask, mask, address[0], p);
			for (int i = 0; i < WordSize; i++)
				_mux(dst[i], lowerMask, data[offset + i], dst[i], p);

			offset += WordSize;
			bit_t upperMask = make_bit(p);
			_and(upperMask, mask, address[0], p);
			for (int i = 0; i < WordSize; i++)
				_mux(dst[i], upperMask, data[offset + i], dst[i], p);
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
		getBits(dst, address.subspan(0, address.size() - 1), dynamicOffset,
		        lowerMask);

		bit_t upperMask = make_bit(p);
		_and(upperMask, mask, address.last(), p);
		getBits(dst, address.subspan(0, address.size() - 1),
		        dynamicOffset + (1 << (address.size() - 1)), upperMask);
	}
};

#endif //FHETOOLS_ARRAY_H
