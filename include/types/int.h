#ifndef GLOVEBOX_INT_H
#define GLOVEBOX_INT_H

#include <cstring>
#include <primitives.h>
#include <serialization.h>
#include <sstream>
#include <structhelper.h>
#include <tfhe.h>
#include <types/type_ids.h>
#include <utility>

// Template programming is ugly.
template <uint8_t N, typename = std::enable_if<N <= 64>>
using smallest_int_t = std::conditional_t<
    (N <= 8), int8_t,
    std::conditional_t<(N <= 16), int16_t, std::conditional_t<(N <= 32), int32_t, int64_t>>>;

template <uint8_t N, typename = std::enable_if<N <= 64>>
using smallest_uint_t = std::conditional_t<
    (N <= 8), uint8_t,
    std::conditional_t<(N <= 16), uint16_t, std::conditional_t<(N <= 32), uint32_t, uint64_t>>>;

template <uint8_t Size> class Int {
  protected:
	using native_type = smallest_int_t<Size>;

  public:
	static const int _wordSize = Size;
	gb::bitvec<Size> data;

	// Create an Int, allocate memory, but do not initialize it
	Int() : data(make_bitvec<Size>()){};
	Int(StructHelper &helper) : data(helper.make_bitvec<Size>()){};

	// Initialize from a plaintext int
	Int(native_type src) : Int() {
		write(src);
	}
	Int(native_type src, StructHelper &helper) : Int(helper) {
		write(src);
	}

	Int(const std::string &packet) : Int() {
		char size_from_header = packet[0];
		assert(size_from_header == Size);
		// Skip header
		std::stringstream ss(packet.substr(1));
		deserialize(ss, data);
	}

	void write(native_type src) {
		for (int i = 0; i < Size; i++)
			::write(data[i], (src >> i) & 1);
	}
	void operator=(native_type src) {
		write(src);
	}


	std::string serialize() const {
		char header[1];
		char mysize = Size;
		memcpy(header, &mysize, 1);
		std::ostringstream oss;
		oss.write(header, sizeof(header));
		::serialize(oss, data);
		return oss.str();
	}

#if !GB_SERVER
	// Decrypts the Int and returns an int of the smallest size possible
	// (5 -> int8_t, 10 -> int16_t, etc)
	native_type toInt() const {
		native_type ret = 0;
		for (int i = 0; i < Size; i++)
			ret |= (::decrypt(data[i]) & 1) << i;
		return ret;
	}
#endif

  protected:
	// Copy a larger bitvec here. Deals with rounding and overflow calculation
	void round_helper(bit_t overflow, const bitvec_t &src, uint8_t truncate_from) {
		_copy(data, src.subspan(truncate_from, Size));
		bit_t sign_bit = src.last();

		if (truncate_from != 0) {
			// We can't just truncate (i.e. _copy): consider the case 0.011,
			// which does not round to 0.01 but to 0.10.
			bit_t should_increment = make_bit();
			_xor(should_increment, sign_bit, src[truncate_from - 1]);
			bitvec_t tmp = make_bitvec(data.size());
			increment_if(tmp, should_increment, data);
			_copy(data, tmp);
		}

		// The number overflows if it is positive and has 1s past what we
		// copied, or it is negative and has 0s past what we copied. (Note that
		// we finished copying at truncate_from + size) It also overflows if the
		// signs of src and data do not match.
		_xor(overflow, sign_bit, data.last());
		for (int i = truncate_from + Size; i < src.size(); i++) {
			bit_t is_overflowing = make_bit();
			_xor(is_overflowing, sign_bit, src[i]);
			overflow |= is_overflowing;
		}
	}
};

class Int8 : public Int<8> {
  public:
	static const int typeID = INT_TYPE_ID;
	static const int _wordSize = 8;

	// Create an Int8, allocate memory, but do not initialize it
	Int8() : Int(){};
	Int8(StructHelper &helper) : Int(helper){};

	// Initialize from a plaintext int8
	Int8(int8_t src) : Int(src){};
	Int8(int8_t src, StructHelper &helper) : Int(src, helper){};
	// Inizialize from a char*
	Int8(const std::string &packet) : Int(packet){};

	void operator=(native_type src) {
		write(src);
	}

	void add(bit_t overflow, Int8 a, Int8 b);
	// Add and do not be notified if overflow happens
	void add(Int8 a, Int8 b);
	// In case of overflow the output will be *truncated* to the 8 LSBs!
	void mul(bit_t overflow, Int8 a, Int8 b, uint8_t truncate_from = 0);
	void mul(Int8 a, Int8 b, uint8_t truncate_from = 0);
	void div(Int8 a, Int8 b);

  private:
	void round(bit_t overflow, const bitvec_t &src, uint8_t truncate_from);
};

class Int16 : public Int<16> {
  public:
	static const int typeID = INT_TYPE_ID;
	static const int _wordSize = 16;

	// Create an Int16, allocate memory, but do not initialize it
	Int16() : Int(){};
	Int16(StructHelper &helper) : Int(helper){};

	// Initialize from a plaintext int16
	Int16(int16_t src) : Int(src){};
	Int16(int16_t src, StructHelper &helper) : Int(src, helper){};
	// Inizialize from a char*
	Int16(const std::string &packet) : Int(packet){};

	void operator=(native_type src) {
		write(src);
	}

	void add(bit_t overflow, Int16 a, Int16 b);
	// Add and do not be notified if overflow happens
	void add(Int16 a, Int16 b);
	// In case of overflow the output will be *truncated* to the 16 LSBs!
	void mul(bit_t overflow, Int16 a, Int16 b, uint8_t truncate_from = 0);
	void mul(Int16 a, Int16 b, uint8_t truncate_from = 0);
	void div(Int16 a, Int16 b);

  private:
	void round(bit_t overflow, const bitvec_t &src, uint8_t truncate_from);
};

// The smallest Int class with at least N bits
template <uint8_t N, typename = std::enable_if<(N <= 16)>>
using smallest_Int = std::conditional_t<(N <= 8), Int8, Int16>;

/*
class Int16 : Int<16> {
public:
    Int16(int16_t);
};
class Int32 : Int<32> {
public:
    Int32(int32_t);
};
*/

#endif // GLOVEBOX_INT_H
