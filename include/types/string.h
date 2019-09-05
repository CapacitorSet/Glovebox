#ifndef GLOVEBOX_STRING_H
#define GLOVEBOX_STRING_H

#include "array.h"
#include "int.h"
#include <cstring>

// A string of fixed length. Note that Length includes the null terminator.
template <uint16_t Length> class String : public Array<Int8, Length> {
  public:
	static constexpr int typeID = STRING_TYPE_ID;

	String(bool initialize_memory = true) : Array<Int8, Length>(initialize_memory) {}
	// The char parameter disambiguates against String(std::string)
	String(const char *src, char) : Array<Int8, Length>(false) {
		const auto len = strlen(src) + 1; // Count trailing NUL
		assert(len <= Length);
		for (size_t i = 0; i < len; i++)
			for (int j = 0; j < 8; j++)
				this->data[i * 8 + j] = (src[i] >> j) & 1;
	}

	String(const std::string &packet) : Array<Int8, Length>() {
		uint16_t length_from_header;
		memcpy(&length_from_header, &packet[0], 2);
		assert(length_from_header == Length);
		// Skip header
		std::stringstream ss(packet.substr(2));
		deserialize(ss, this->data);
	}

	void toCStr(char *dst) const {
		for (size_t i = 0; i < Length; i++) {
			char out = 0;
			for (int j = 0; j < 8; j++)
				out |= decrypt(this->data[i * 8 + j]) << j;
			dst[i] = out;
		}
	}

	std::string serialize() const {
		char header[2];
		uint16_t size = Length;
		memcpy(header, &size, 2);
		std::ostringstream oss;
		oss.write(header, sizeof(header));
		::serialize(oss, this->data);
		return oss.str();
	}
};

#endif // GLOVEBOX_STRING_H
