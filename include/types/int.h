#include <utility>

#ifndef FHETOOLS_INT_H
#define FHETOOLS_INT_H

#include <primitives.h>
#include <sstream>
#include <tfhe.h>
#include <types/type_ids.h>
#include <serialization.h>
#include <structhelper.h>

// Template programming is ugly.
template<uint8_t N, typename = std::enable_if<N <= 64>>
using smallest_int_t =
	std::conditional_t<(N <= 8), int8_t,
	std::conditional_t<(N <= 16), int16_t,
	std::conditional_t<(N <= 32), int32_t,
	int64_t
>>>;

template<uint8_t N, typename = std::enable_if<N <= 64>>
using smallest_uint_t =
	std::conditional_t<(N <= 8), uint8_t,
	std::conditional_t<(N <= 16), uint16_t,
	std::conditional_t<(N <= 32), uint32_t,
	uint64_t
>>>;

template <uint8_t size>
class Int {
private:
	using native_type_t = smallest_int_t<size>;

protected:
	TFHEServerParams_t p;

	Int() = delete;
	// Create an Int, allocate memory, but do not initialize it
	explicit Int(TFHEServerParams_t _p)
		: p(_p), data(make_bitspan<size>(_p)) {};
	Int(StructHelper &helper, TFHEServerParams_t _p)
		: p(_p), data(helper.make_bitspan<size>(p)) {};

	// Initialize from a plaintext int
	Int(native_type_t src, only_TFHEServerParams_t _p = default_server_params)
		: Int(unwrap_only(_p)) {
		constant(src, _p);
	}
	Int(native_type_t src, StructHelper &helper, only_TFHEServerParams_t _p = default_server_params)
		: Int(helper, unwrap_only(_p)) {
		constant(src, _p);
	}
	Int(native_type_t src, TFHEClientParams_t _p) : Int(_p) {
		encrypt(src, _p);
	}
	Int(native_type_t src, StructHelper &helper, TFHEClientParams_t _p)
		: Int(helper, _p) {
		encrypt(src, _p);
	}

	Int(const std::string &packet, TFHEServerParams_t _p = default_server_params) : Int(_p) {
		char size_from_header = packet[0];
		assert(size_from_header == size);
		// Skip header
		std::stringstream ss(packet.substr(1));
		deserialize(ss, data, p);
	}

public:
	void encrypt(native_type_t src, TFHEClientParams_t _p = default_client_params) {
		for (int i = 0; i < size; i++)
			::encrypt(data[i], (src >> i) & 1, _p);
	}
	// The only_TFHEServerParams_t param is not strictly necessary, but it helps
	// against accidental API misuse.
	void constant(native_type_t src, only_TFHEServerParams_t _p = default_server_params) {
		for (int i = 0; i < size; i++)
			::constant(data[i], (src >> i) & 1, _p);
	}

	std::string exportToString() {
		// Todo: header should have >1 byte for size
		char header[1];
		char mysize = size;
		memcpy(header, &mysize, 1);
		std::ostringstream oss;
		oss.write(header, sizeof(header));
		serialize(oss, data, p);
		return oss.str();
	}

	// Decrypts the Int and returns an int of the smallest size possible
	// (5 -> int8_t, 10 -> int16_t, etc)
	native_type_t toInt(TFHEClientParams_t p = default_client_params) {
		native_type_t ret = 0;
		for (int i = 0; i < size; i++)
			ret |= (::decrypt(data[i], p) & 1) << i;
		return ret;
	}
public:
	fixed_bitspan_t<size> data;
};

class Int8 : public Int<8> {
public:
	static const int typeID = INT_TYPE_ID;
	static const int _wordSize = 8;

	Int8() = delete;
	// Create an Int8, allocate memory, but do not initialize it
	explicit Int8(TFHEServerParams_t _p) : Int(_p) {};
	Int8(StructHelper &helper, TFHEServerParams_t _p)
		: Int(helper, _p) {};

	// Initialize from a plaintext int8
	Int8(int8_t src, only_TFHEServerParams_t _p = default_server_params)
		: Int(src, _p) {};
	Int8(int8_t src, StructHelper &helper, only_TFHEServerParams_t _p = default_server_params)
		: Int(src, helper, _p) {};
	Int8(int8_t src, TFHEClientParams_t _p)
		: Int(src, _p) {};
	Int8(int8_t src, StructHelper &helper, TFHEClientParams_t _p)
		: Int(src, helper, _p) {};
	// Inizialize from a char*
	Int8(const std::string &packet, TFHEServerParams_t _p = default_server_params)
		: Int(packet, _p) {};

	void add(bit_t overflow, Int8 a, Int8 b);
	// Add and do not be notified if overflow happens
	void add(Int8 a, Int8 b);
	void increment_if(bit_t cond);
	// In case of overflow the output will be *truncated* to the 8 LSBs!
	void mul(bit_t overflow, Int8 a, Int8 b, uint8_t truncate_from = 0);
	void mul(Int8 a, Int8 b, uint8_t truncate_from = 0);

	void copy(Int8 src);
};

class Int16 : public Int<16> {
public:
	static const int typeID = INT_TYPE_ID;
	static const int _wordSize = 16;

	Int16() = delete;
	// Create an Int16, allocate memory, but do not initialize it
	explicit Int16(TFHEServerParams_t _p) : Int(_p) {};
	Int16(StructHelper &helper, TFHEServerParams_t _p)
		: Int(helper, _p) {};

	// Initialize from a plaintext int16
	Int16(int16_t src, only_TFHEServerParams_t _p = default_server_params)
		: Int(src, _p) {};
	Int16(int16_t src, StructHelper &helper, only_TFHEServerParams_t _p = default_server_params)
		: Int(src, helper, _p) {};
	Int16(int16_t src, TFHEClientParams_t _p)
		: Int(src, _p) {};
	Int16(int16_t src, StructHelper &helper, TFHEClientParams_t _p)
		: Int(src, helper, _p) {};
	// Inizialize from a char*
	Int16(const std::string &packet, TFHEServerParams_t _p = default_server_params)
		: Int(packet, _p) {};

	void add(bit_t overflow, Int16 a, Int16 b);
	// Add and do not be notified if overflow happens
	void add(Int16 a, Int16 b);
	void increment_if(bit_t cond);
	// In case of overflow the output will be *truncated* to the 16 LSBs!
	void mul(bit_t overflow, Int16 a, Int16 b, uint16_t truncate_from = 0);
	void mul(Int16 a, Int16 b, uint16_t truncate_from = 0);

	void copy(Int16 src);
};

// The smallest Int class with at least N bits
template<uint8_t N>
using smallest_Int = typename std::enable_if<N <= 16,
	std::conditional_t<(N <= 8), Int8, Int16>
>::type; // If you're reading this, you're requesting too large of an Int.

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

#endif //FHETOOLS_INT_H
