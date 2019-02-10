#ifndef FHETOOLS_STRING_H
#define FHETOOLS_STRING_H

#include <cstring>
#include "array.h"
#include "int.h"
#include "tfhe.h"

// A string of *fixed* length.
class String : protected Array<Int> {
public:
	explicit String(uint16_t len, TFHEServerParams_t p = default_server_params) : Array<Int>(len, 8, p) {}
	explicit String(char *src, TFHEServerParams_t p = default_server_params) : Array<Int>(strlen(src), 8, p) {
		for (size_t i = 0; i < length; i++)
			for (int j = 0; j < 8; j++)
				constant(data[i * 8 + j], src[i], p);
	}

	void toCStr(char *dst, TFHEClientParams_t _p = default_client_params) {
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
	ClientString(uint16_t len, TFHEClientParams_t p = default_client_params) : String(len, p) {}
	ClientString(char *src, TFHEClientParams_t p = default_client_params) : String(src, p) {
		for (size_t i = 0; i < length; i++)
			for (int j = 0; j < 8; j++)
				encrypt(data[i * 8 + j], (src[i] >> j) & 1, p);
	}
};

#endif //FHETOOLS_STRING_H
