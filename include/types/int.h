#ifndef FHETOOLS_INT_H
#define FHETOOLS_INT_H

#include <primitives.h>
#include <sstream>
#include <tfhe.h>
#include <types/type_ids.h>
#include <serialization.h>

// Template programming is ugly.
template<uint8_t N, typename = std::enable_if<N <= 64>>
using smallest_int_t =
	std::conditional_t<(N <= 8), int8_t,
	std::conditional<(N <= 16), int16_t,
	std::conditional<(N <= 32), int32_t,
	int64_t
>>>;

template <uint8_t size>
class Int {
	using native_type_t = smallest_int_t<size>;
public:
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
protected:
	// Create an Int8, but do not initialize the memory
	explicit Int(TFHEServerParams_t _p = default_server_params)
		: p(_p), data(make_fixed_bitspan<size>(_p)) {};

	// Initialize from a plaintext int
	explicit Int(native_type_t src, TFHEServerParams_t _p = default_server_params)
		: Int(_p) {
		for (int i = 0; i < size; i++)
			constant(data[i], (src >> i) & 1, _p);
	}

	// Initialize from a char*
	Int(const char *packet, size_t pktsize, TFHEServerParams_t _p) : Int(_p) {
		char size_from_header;
		memcpy(&size_from_header, packet, 1);
		assert(size_from_header == size);
		// Skip header
		packet += 1;
		pktsize -= 1;
		std::stringstream ss;
		ss.write(packet, pktsize);
		deserialize(ss, data, p);
	}

	TFHEServerParams_t p;
	fixed_bitspan_t<size> data;
};

class Int8 : public Int<8> {
	template <class T> friend class Array; // Int8.data is used to access arrays
public:
	static const int typeID = INT_TYPE_ID;

	// Create an Int8, but do not initialize the memory
	explicit Int8(TFHEServerParams_t _p = default_server_params)
		: Int(_p) {};
	// Initialize from a plaintext int8
	explicit Int8(int8_t src, TFHEServerParams_t _p = default_server_params)
		: Int(src, _p) {};
	// Inizialize from a char*
	Int8(const char *packet, size_t pktsize, TFHEServerParams_t _p = default_server_params)
		: Int(packet, pktsize, _p) {};

	void add(bit_t overflow, Int8 a, Int8 b);
	// Add and do not be notified if overflow happens
	void add(Int8 a, Int8 b);
	// In case of overflow the output will be *truncated* to the 8 LSBs!
	void mul(bit_t overflow, Int8 a, Int8 b, uint8_t truncate_from = 0);
	void mul(Int8 a, Int8 b, uint8_t truncate_from = 0);

	// Used for array stuff; to be implemented later
	/*
	void _writeTo(bitspan_t dst) { _copy(dst, data, p); }
	void _fromBytes(bitspan_t dst) { _copy(data, dst, p); }
	maskable_function_t _m_fromBytes(bitspan_t dst) {
		return [=] (bit_t mask) -> void {
			for (int i = 0; i < data.size(); i++)
				_mux(data[i], mask, dst[i], data[i], p);
		};
	}
	*/

	void copy(Int8 src);
};

// The smallest Int class with at least N bits
template<uint8_t N, typename = std::enable_if<N <= 8>>
using smallest_Int = std::conditional_t<(N <= 8), Int8, /* error! */ void>;
// When Int16, etc. are implemented it will have several std::conditional
// clauses, one for each

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
