#ifndef FHETOOLS_FIXED32_H
#define FHETOOLS_FIXED32_H

#include <types/int.h>

// Are you seeing the error "Base specifier must name a class" at this line?
// Then the size of your fixed is too large.
#define BASE_INT smallest_Int<INT_SIZE + FRAC_SIZE>
template <uint8_t INT_SIZE, uint8_t FRAC_SIZE>
class Fixed : public BASE_INT {
	static const int SIZE = INT_SIZE + FRAC_SIZE;
	static_assert(SIZE <= 8, "Size not supported");
	using native_type_t = smallest_int_t<SIZE>;

	// Scale the number and return it as an integer
	static native_type_t scale(double src) {
		double scaled = round(src * double(1 << FRAC_SIZE));
		// Assert that the scaled number fits
		if (scaled > double(std::numeric_limits<native_type_t>::max())) {
			double upper_limit = std::numeric_limits<native_type_t>::max() >> FRAC_SIZE;
			fprintf(stderr, "Value is too high: Fixed<%d,%d> was initialized with %lf, but maximum is %lf\n", INT_SIZE, FRAC_SIZE, src, upper_limit);
			abort();
		}
		if (scaled < double(std::numeric_limits<native_type_t>::min())) {
			double lower_limit = std::numeric_limits<native_type_t>::min() >> FRAC_SIZE;
			fprintf(stderr, "Value is too low: Fixed<%d,%d> was initialized with %lf, but minimum is %lf\n", INT_SIZE, FRAC_SIZE, src, lower_limit);
			abort();
		}
		return native_type_t(scaled);
	}

public:
	static const int typeID = FIXED_TYPE_ID;
	static const int _wordSize = INT_SIZE + FRAC_SIZE;

	Fixed() = delete;
	explicit Fixed(TFHEServerParams_t _p)
		: BASE_INT(_p) {};
	explicit Fixed(StructHelper &helper, TFHEServerParams_t _p = default_server_params)
		: BASE_INT(helper, _p) {};
	Fixed(double src, only_TFHEServerParams_t _p = default_server_params)
		: BASE_INT(scale(src), unwrap_only(_p)) {};
	Fixed(double src, StructHelper &helper, only_TFHEServerParams_t _p = default_server_params)
		: BASE_INT(scale(src), helper, unwrap_only(_p)) {};
	Fixed(double src, TFHEClientParams_t _p)
		: BASE_INT(scale(src), _p) {};
	Fixed(double src, StructHelper &helper, TFHEClientParams_t _p)
		: BASE_INT(scale(src), helper, _p) {};

	Fixed(const std::string &packet, TFHEServerParams_t _p = default_server_params)
		: Fixed<INT_SIZE, FRAC_SIZE>(_p) {
		char int_size_from_header = packet[0];
		char frac_size_from_header = packet[1];
		assert(int_size_from_header == INT_SIZE);
		assert(frac_size_from_header == FRAC_SIZE);
		// Skip header
		std::stringstream ss(packet.substr(2));
		deserialize(ss, this->data, this->p);
	}

	void encrypt(double src, TFHEClientParams_t _p) {
		BASE_INT::encrypt(scale(src), _p);
	}
	void constant(double src, only_TFHEServerParams_t _p) {
		BASE_INT::constant(scale(src), _p);
	}

	void add(bit_t overflow, Fixed<INT_SIZE, FRAC_SIZE> a, Fixed<INT_SIZE, FRAC_SIZE> b) {
		BASE_INT::add(overflow, a, b);
	}
	void mul(bit_t overflow, Fixed<INT_SIZE, FRAC_SIZE> a, Fixed<INT_SIZE, FRAC_SIZE> b) {
		BASE_INT::mul(overflow, a, b, FRAC_SIZE);
	}

	double toDouble(TFHEClientParams_t p = default_client_params) {
		native_type_t tmp = this->toInt(p);
		return double(tmp) / (1 << FRAC_SIZE);
	};
	std::string exportToString() {
		char header[2] = {INT_SIZE, FRAC_SIZE};
		std::ostringstream oss;
		oss.write(header, sizeof(header));
		serialize(oss, this->data, this->p);
		return oss.str();
	}

	void copy(Fixed<INT_SIZE, FRAC_SIZE> src) {
		for (int i = 0; i < this->data.size(); i++)
			_copy(this->data[i], src.data[i], this->p);
	}
};

#endif //FHETOOLS_FIXED32_H
