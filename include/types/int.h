#ifndef FHETOOLS_INT_H
#define FHETOOLS_INT_H

#include <primitives.h>
#include <sstream>
#include <tfhe.h>
#include <types/type_ids.h>
#include <serialization.h>

template <uint8_t size>
class Int {
public:
	explicit Int(TFHEServerParams_t _p = default_server_params)
			: p(_p), data(make_fixed_bitspan<size>(_p)) {};

	std::string exportToString() {
		// Todo: header should have >1 byte for size
		char header[1];
		char mysize = size;
		memcpy(header, &mysize, 1);
		std::ostringstream oss;
		oss << header;
		serialize(oss, data, p);
		return oss.str();
	}
protected:
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
	explicit Int8(int8_t src, TFHEServerParams_t _p = default_server_params) : Int(_p) {
		for (int i = 0; i < 8; i++)
			constant(data[i], (src >> i) & 1, _p);
	}
	// Inizialize from a char*
	Int8(const char *packet, size_t pktsize, TFHEServerParams_t _p = default_server_params)
		: Int(packet, pktsize, _p) {};

	void add(Int8 a, Int8 b);
	/*
	void mul(Int8 a, Int8 b) {
		::mul8(data, a.data, b.data, p);
	}
	*/

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

	int8_t toI8(TFHEClientParams_t p = default_client_params);
};
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
