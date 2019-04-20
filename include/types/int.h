#ifndef FHETOOLS_INT_H
#define FHETOOLS_INT_H

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
    std::conditional_t<(N <= 16), int16_t,
                       std::conditional_t<(N <= 32), int32_t, int64_t>>>;

template <uint8_t N, typename = std::enable_if<N <= 64>>
using smallest_uint_t = std::conditional_t<
    (N <= 8), uint8_t,
    std::conditional_t<(N <= 16), uint16_t,
                       std::conditional_t<(N <= 32), uint32_t, uint64_t>>>;

template <uint8_t size> class Int {
	using native_type_t = smallest_int_t<size>;

  protected:
	weak_params_t p;

  public:
	static const int _wordSize = size;
	fixed_bitspan_t<size> data;

	Int() = delete;
	// Create an Int, allocate memory, but do not initialize it
	explicit Int(weak_params_t _p) : p(_p), data(make_bitspan<size>(_p)){};
	Int(StructHelper &helper, weak_params_t _p)
	    : p(_p), data(helper.make_bitspan<size>(p)){};

	// Initialize from a plaintext int
	Int(native_type_t src, TFHEServerParams_t _p) : Int(_p) {
		constant(src, _p);
	}
	Int(native_type_t src, StructHelper &helper, TFHEServerParams_t _p)
	    : Int(helper, _p) {
		constant(src, _p);
	}
	Int(native_type_t src, TFHEClientParams_t _p = default_client_params)
	    : Int(_p) {
		encrypt(src, _p);
	}
	Int(native_type_t src, StructHelper &helper,
	    TFHEClientParams_t _p = default_client_params)
	    : Int(helper, _p) {
		encrypt(src, _p);
	}

	Int(const std::string &packet, weak_params_t _p = default_weak_params)
	    : Int(_p) {
		char size_from_header = packet[0];
		assert(size_from_header == size);
		// Skip header
		std::stringstream ss(packet.substr(1));
		deserialize(ss, data, p);
	}

	void encrypt(native_type_t src,
	             TFHEClientParams_t _p = default_client_params) {
		for (int i = 0; i < size; i++)
			::encrypt(data[i], (src >> i) & 1, _p);
	}
	void constant(native_type_t src,
	              TFHEServerParams_t _p = default_server_params) {
		for (int i = 0; i < size; i++)
			::constant(data[i], (src >> i) & 1, _p);
	}

	std::string serialize() const {
		char header[1];
		char mysize = size;
		memcpy(header, &mysize, 1);
		std::ostringstream oss;
		oss.write(header, sizeof(header));
		::serialize(oss, data, p);
		return oss.str();
	}

	// Decrypts the Int and returns an int of the smallest size possible
	// (5 -> int8_t, 10 -> int16_t, etc)
	native_type_t toInt(TFHEClientParams_t p = default_client_params) const {
		native_type_t ret = 0;
		for (int i = 0; i < size; i++)
			ret |= (::decrypt(data[i], p) & 1) << i;
		return ret;
	}

	// Convenience method. Makes for more readable code.
	bit_t isNegative() const { return data.last(); }

  protected:
	// Copy a larger bitspan here. Deals with rounding and overflow calculation
	void round_helper(bit_t overflow, const bitspan_t &src,
	                  uint8_t truncate_from) {
		_copy(data, src.subspan(truncate_from, size), p);
		bit_t sign_bit = src.last();

		if (truncate_from != 0) {
			// We can't just truncate (i.e. _copy): consider the case 0.011,
			// which does not round to 0.01 but to 0.10.
			bit_t should_increment = make_bit(p);
			_xor(should_increment, sign_bit, src[truncate_from - 1], p);
			bitspan_t tmp = make_bitspan(data.size(), p);
			incr_if(tmp, should_increment, data, p);
			_copy(data, tmp, p);
		}

		// The number overflows if it is positive and has 1s past what we
		// copied, or it is negative and has 0s past what we copied. (Note that
		// we finished copying at truncate_from + size) It also overflows if the
		// signs of src and data do not match.
		_xor(overflow, sign_bit, data.last(), p);
		for (int i = truncate_from + size; i < src.size(); i++) {
			bit_t is_overflowing = make_bit(p);
			_xor(is_overflowing, sign_bit, src[i], p);
			_or(overflow, overflow, is_overflowing, p);
		}
	}
};

class Int8 : public Int<8> {
  public:
	static const int typeID = INT_TYPE_ID;
	static const int _wordSize = 8;

	Int8() = delete;
	// Create an Int8, allocate memory, but do not initialize it
	explicit Int8(weak_params_t _p) : Int(_p){};
	Int8(StructHelper &helper, weak_params_t _p) : Int(helper, _p){};

	// Initialize from a plaintext int8
	Int8(int8_t src, TFHEServerParams_t _p) : Int(src, _p){};
	Int8(int8_t src, StructHelper &helper, TFHEServerParams_t _p)
	    : Int(src, helper, _p){};
	Int8(int8_t src, TFHEClientParams_t _p = default_client_params)
	    : Int(src, _p){};
	Int8(int8_t src, StructHelper &helper,
	     TFHEClientParams_t _p = default_client_params)
	    : Int(src, helper, _p){};
	// Inizialize from a char*
	Int8(const std::string &packet, weak_params_t _p = default_weak_params)
	    : Int(packet, _p){};

	void add(bit_t overflow, Int8 a, Int8 b);
	// Add and do not be notified if overflow happens
	void add(Int8 a, Int8 b);
	// In case of overflow the output will be *truncated* to the 8 LSBs!
	void mul(bit_t overflow, Int8 a, Int8 b, uint8_t truncate_from = 0);
	void mul(Int8 a, Int8 b, uint8_t truncate_from = 0);
	void div(Int8 a, Int8 b);

	void copy(const Int8 src);

	bit_t is_zero() const;
	bit_t is_nonzero() const;
	void increment_if(bit_t cond);
	void decrement_if(bit_t cond);

  private:
	void round(bit_t overflow, const bitspan_t &src, uint8_t truncate_from);
};

class Int16 : public Int<16> {
  public:
	static const int typeID = INT_TYPE_ID;
	static const int _wordSize = 16;

	Int16() = delete;
	// Create an Int16, allocate memory, but do not initialize it
	explicit Int16(weak_params_t _p) : Int(_p){};
	Int16(StructHelper &helper, weak_params_t _p) : Int(helper, _p){};

	// Initialize from a plaintext int16
	Int16(int16_t src, TFHEServerParams_t _p) : Int(src, _p){};
	Int16(int16_t src, StructHelper &helper, TFHEServerParams_t _p)
	    : Int(src, helper, _p){};
	Int16(int16_t src, TFHEClientParams_t _p = default_client_params)
	    : Int(src, _p){};
	Int16(int16_t src, StructHelper &helper,
	      TFHEClientParams_t _p = default_client_params)
	    : Int(src, helper, _p){};
	// Inizialize from a char*
	Int16(const std::string &packet, weak_params_t _p = default_weak_params)
	    : Int(packet, _p){};

	void add(bit_t overflow, Int16 a, Int16 b);
	// Add and do not be notified if overflow happens
	void add(Int16 a, Int16 b);
	// In case of overflow the output will be *truncated* to the 16 LSBs!
	void mul(bit_t overflow, Int16 a, Int16 b, uint8_t truncate_from = 0);
	void mul(Int16 a, Int16 b, uint8_t truncate_from = 0);
	void div(Int16 a, Int16 b);

	void copy(Int16 src);

	bit_t is_zero() const;
	bit_t is_nonzero() const;
	void increment_if(bit_t cond);
	void decrement_if(bit_t cond);

  private:
	void round(bit_t overflow, const bitspan_t &src, uint8_t truncate_from);
};

// The smallest Int class with at least N bits
template <uint8_t N>
using smallest_Int =
    typename std::enable_if<N <= 16,
                            std::conditional_t<(N <= 8), Int8, Int16>>::
        type; // If you're reading this, you're requesting too large of an Int.

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

#endif // FHETOOLS_INT_H
