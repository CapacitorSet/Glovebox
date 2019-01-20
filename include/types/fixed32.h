#ifndef FHETOOLS_FIXED32_H
#define FHETOOLS_FIXED32_H

#include <cstdint>
#include <tfhe.h>
#include <primitives.h>

// 16 bit integer part + 16 bit fractional
class Fixed32 {
public:
	Fixed32(float src, TFHEServerParams_t _p) : Fixed32(_p) {
		int32_t tmp = (int32_t) src; // Extract integer part
		src -= tmp;
		tmp <<= 16;
		tmp += (int32_t) (src * (1 << 16));

		for (int i = 0; i < 32; i++)
			constant(data[i], (tmp >> i) & 1, p);
	}

	float toFloat(TFHEClientParams_t _p);

	Fixed32 plus(Fixed32 src);
	Fixed32 times(Fixed32 src);

	void free() {
		free_bitspan(data);
	}

protected:
	explicit Fixed32(TFHEServerParams_t _p) : p(_p) {
		data = make_bitspan(32, p);
	}

	bitspan_t data;
	TFHEServerParams_t p;
};

class ClientFixed32 : public Fixed32 {
public:
	ClientFixed32(float src, TFHEClientParams_t _p) : Fixed32(makeTFHEServerParams(_p)) {
		data = make_bitspan(32, p);

		int32_t tmp = (int32_t) src; // Extract integer part
		src -= tmp;
		tmp <<= 16;
		tmp += (int32_t) (src * (1 << 16));

		for (int i = 0; i < 32; i++)
			constant(data[i], (tmp >> i) & 1, p);
	}
};

#endif //FHETOOLS_FIXED32_H
