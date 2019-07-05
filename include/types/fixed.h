#ifndef GLOVEBOX_FIXED32_H
#define GLOVEBOX_FIXED32_H

#include <cmath>
#include <types/int.h>

// Are you seeing the error "Base specifier must name a class" at this line?
// Then the size of your fixed is too large.
#define BaseInt smallest_Int<IntSize + FracSize>
template <uint8_t IntSize, uint8_t FracSize> class Fixed : public BaseInt {
	static const int SIZE = IntSize + FracSize;
	static_assert(SIZE <= 16, "Size not supported");

	using native_type_t = smallest_int_t<SIZE>;
	// Numeric limits of underlying storage
	static constexpr int64_t native_max =
	    std::numeric_limits<native_type_t>::max();
	static constexpr int64_t native_min =
	    std::numeric_limits<native_type_t>::min();
	// Scale the number and return it as an integer
	static native_type_t scale(double src) {
		double scaled = round(src * double(1 << FracSize));
		// Assert that the scaled number fits
		if (scaled > double(native_max)) {
			double upper_limit = native_max >> FracSize;
			fprintf(stderr,
			        "Value is too high: Fixed<%d,%d> was initialized with %lf, "
			        "but maximum is %lf\n",
			        IntSize, FracSize, src, upper_limit);
			abort();
		}
		if (scaled < double(native_min)) {
			double lower_limit = native_min >> FracSize;
			fprintf(stderr,
			        "Value is too low: Fixed<%d,%d> was initialized with %lf, "
			        "but minimum is %lf\n",
			        IntSize, FracSize, src, lower_limit);
			abort();
		}
		return native_type_t(scaled);
	}

	static constexpr double undo_scale(native_type_t src) {
		return double(src) / (1 << FracSize);
	}

  public:
	// Numeric limits of representable fixeds
	static constexpr double max = undo_scale(native_max);
	static constexpr double min = undo_scale(native_min);

	static const uint8_t intSize = IntSize;
	static const uint8_t fracSize = FracSize;
	static const int typeID = FIXED_TYPE_ID;
	static const int _wordSize = IntSize + FracSize;

	Fixed() = delete;
	explicit Fixed(WeakParams _p) : BaseInt(_p){};
	explicit Fixed(StructHelper &helper, WeakParams _p = default_weak_params)
	    : BaseInt(helper, _p){};
	Fixed(double src, ServerParams _p) : BaseInt(scale(src), _p){};
	Fixed(double src, StructHelper &helper, ServerParams _p)
	    : BaseInt(scale(src), helper, _p){};
	Fixed(double src, ClientParams _p = default_client_params)
	    : BaseInt(scale(src), _p){};
	Fixed(double src, StructHelper &helper,
	      ClientParams _p = default_client_params)
	    : BaseInt(scale(src), helper, _p){};

	Fixed(const std::string &packet, WeakParams _p = default_weak_params)
	    : Fixed<IntSize, FracSize>(_p) {
		char intSize_from_header = packet[0];
		char fracSize_from_header = packet[1];
		assert(intSize_from_header == IntSize);
		assert(fracSize_from_header == FracSize);
		// Skip header
		std::stringstream ss(packet.substr(2));
		deserialize(ss, this->data, this->p);
	}

	void encrypt(double src, ClientParams _p) {
		BaseInt::encrypt(scale(src), _p);
	}
	void constant(double src, ServerParams _p) {
		BaseInt::constant(scale(src), _p);
	}

	void add(bit_t overflow, Fixed<IntSize, FracSize> a,
	         Fixed<IntSize, FracSize> b) {
		BaseInt::add(overflow, a, b);
	}
	void mul(bit_t overflow, Fixed<IntSize, FracSize> a,
	         Fixed<IntSize, FracSize> b) {
		BaseInt::mul(overflow, a, b, FracSize);
	}

	double toDouble(ClientParams p = default_client_params) const {
		return undo_scale(this->toInt(p));
	};

	std::string serialize() const {
		char header[2] = {IntSize, FracSize};
		std::ostringstream oss;
		oss.write(header, sizeof(header));
		::serialize(oss, this->data, this->p);
		return oss.str();
	}

	void copy(Fixed<IntSize, FracSize> src) {
		for (int i = 0; i < this->data.size(); i++)
			_copy(this->data[i], src.data[i], this->p);
	}
};

// Sign-extend a fixed into a larger one
template <uint8_t INT_NEW, uint8_t INT_OLD, uint8_t FracSize>
Fixed<INT_NEW, FracSize> fixed_extend(Fixed<INT_OLD, FracSize> src,
                                      WeakParams _p) {
	static_assert(INT_NEW >= INT_OLD);
	Fixed<INT_NEW, FracSize> ret(_p);
	bit_t sign = src.data.last();
	for (int i = 0; i < ret.data.size(); i++) {
		if (i < src.data.size())
			_copy(ret.data[i], src.data[i], _p);
		else
			_copy(ret.data[i], sign, _p);
	}
	return ret;
}

#endif // GLOVEBOX_FIXED32_H
