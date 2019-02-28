#ifndef FHETOOLS_STRING_H
#define FHETOOLS_STRING_H

#include <cstring>
#include "array.h"
#include "int.h"

// A string of *fixed* length.
template <uint16_t Length>
class String : public Array<Int8, Length> {
public:
	static constexpr int typeID = STRING_TYPE_ID;

	explicit String(char /*disambiguation param*/, bool initialize_memory = true, only_TFHEServerParams_t _p = default_server_params)
		: Array<Int8, Length>(initialize_memory, _p) {}
	String(char /*disambiguation param*/, bool initialize_memory, TFHEClientParams_t _p)
		: Array<Int8, Length>(initialize_memory, _p) {}
	// The bool disambiguates against the deserialization ctor
	explicit String(const char *src, bool, only_TFHEServerParams_t _p = default_server_params)
		: Array<Int8, Length>(true, _p) {
		assert(strlen(src) <= Length);
		const auto len = strlen(src);
		for (size_t i = 0; i < len; i++)
			for (int j = 0; j < 8; j++)
				constant(this->data[i * 8 + j], (src[i] >> j) & 1, _p);
	}
	explicit String(const char *src, bool, TFHEClientParams_t _p)
		: Array<Int8, Length>(true, _p) {
		assert(strlen(src) <= Length);
		const auto len = strlen(src);
		for (size_t i = 0; i < len; i++)
			for (int j = 0; j < 8; j++)
				encrypt(this->data[i * 8 + j], (src[i] >> j) & 1, _p);
	}

	String(const std::string &packet, TFHEServerParams_t _p = default_server_params)
		: Array<Int8, Length>(false, _p) {
		uint16_t length_from_header;
		memcpy(&length_from_header, &packet[0], 2);
		assert(length_from_header == Length);
		// Skip header
		std::stringstream ss(packet.substr(2));
		deserialize(ss, this->data, this->p);
	}

	void toCStr(char *dst, TFHEClientParams_t _p = default_client_params) {
		for (size_t i = 0; i < Length; i++) {
			char out = 0;
			for (int j = 0; j < 8; j++)
				out |= decrypt(this->data[i * 8 + j], _p) << j;
			dst[i] = out;
		}
	}

	std::string exportToString() {
		char header[2];
		uint16_t size = Length;
		memcpy(header, &size, 2);
		std::ostringstream oss;
		oss.write(header, sizeof(header));
		serialize(oss, this->data, this->p);
		return oss.str();
	}
};

#endif //FHETOOLS_STRING_H
