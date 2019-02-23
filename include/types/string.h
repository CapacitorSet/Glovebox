#ifndef FHETOOLS_STRING_H
#define FHETOOLS_STRING_H

#include <cstring>
#include "array.h"
#include "int.h"
/*
// A string of *fixed* length.
class String : protected Array<Int8> {
public:
	explicit String(uint16_t len, TFHEServerParams_t p = default_server_params) : Array<Int8>(len, 8, p) {}
	explicit String(char *src, TFHEServerParams_t p = default_server_params) : Array<Int8>(strlen(src), 8, p) {
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
 */

#endif //FHETOOLS_STRING_H
