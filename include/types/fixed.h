#ifndef GLOVEBOX_FIXED32_H
#define GLOVEBOX_FIXED32_H

#include <cmath>
#include <types/int.h>

template <uint8_t IntSize, uint8_t FracSize, class = std::enable_if_t<(IntSize + FracSize) <= 16>>
class Fixed : public smallest_Int<IntSize + FracSize> {
	using BaseInt = smallest_Int<IntSize + FracSize>;

	// Numeric limits of underlying storage
	using native_type = typename BaseInt::native_type;
	static constexpr int64_t native_max = std::numeric_limits<native_type>::max();
	static constexpr int64_t native_min = std::numeric_limits<native_type>::min();

	// Scale the number and return it as an integer
	static native_type double_to_native(double src) {
		double scaled = round(src * double(1 << FracSize));
		// Assert that the scaled number fits
		if (scaled > double(native_max)) {
			double upper_limit = native_max >> FracSize;
			fprintf(
			    stderr,
			    "Value is too high: Fixed<%d,%d> was initialized with %lf, but maximum is %lf\n",
			    IntSize, FracSize, src, upper_limit);
			abort();
		}
		if (scaled < double(native_min)) {
			double lower_limit = native_min >> FracSize;
			fprintf(stderr,
			        "Value is too low: Fixed<%d,%d> was initialized with %lf, but minimum is %lf\n",
			        IntSize, FracSize, src, lower_limit);
			abort();
		}
		return native_type(scaled);
	}

	static constexpr double native_to_double(native_type src) {
		return double(src) / (1 << FracSize);
	}

  public:
	// Numeric limits of representable fixeds
	static constexpr double max = native_to_double(native_max);
	static constexpr double min = native_to_double(native_min);

	static const uint8_t int_size = IntSize;
	static const uint8_t frac_size = FracSize;
	static const int typeID = FIXED_TYPE_ID;
	static const int _wordSize = IntSize + FracSize;

	Fixed() : BaseInt(){};
	Fixed(StructHelper &helper) : BaseInt(helper){};
	Fixed(double src) : BaseInt(double_to_native(src)){};
	Fixed(double src, StructHelper &helper) : BaseInt(double_to_native(src), helper){};

	Fixed(const std::string &packet) : Fixed<IntSize, FracSize>() {
		char intSize_from_header = packet[0];
		char fracSize_from_header = packet[1];
		assert(intSize_from_header == IntSize);
		assert(fracSize_from_header == FracSize);
		// Skip header
		std::stringstream ss(packet.substr(2));
		deserialize(ss, this->data);
	}

	void encrypt(double src) {
		BaseInt::encrypt(double_to_native(src));
	}
	void constant(double src) {
		BaseInt::constant(double_to_native(src));
	}

	void add(bit_t overflow, Fixed<IntSize, FracSize> a, Fixed<IntSize, FracSize> b) {
		BaseInt::add(overflow, a, b);
	}
	void mul(bit_t overflow, Fixed<IntSize, FracSize> a, Fixed<IntSize, FracSize> b) {
		BaseInt::mul(overflow, a, b, FracSize);
	}

	double toDouble() const {
		return native_to_double(this->toInt());
	};

	std::string serialize() const {
		char header[2] = {IntSize, FracSize};
		std::ostringstream oss;
		oss.write(header, sizeof(header));
		::serialize(oss, this->data);
		return oss.str();
	}

	void copy(Fixed<IntSize, FracSize> src) {
		for (int i = 0; i < this->data.size(); i++)
			_copy(this->data[i], src.data[i]);
	}
};

// Sign-extend a fixed into a larger one
template <uint8_t IntNew, uint8_t FracNew, uint8_t IntOld, uint8_t FracOld>
Fixed<IntNew, FracNew> fixed_extend(Fixed<IntOld, FracOld> src) {
	static_assert(IntNew >= IntOld);
	static_assert(FracNew == FracOld, "not yet implemented");
	Fixed<IntNew, FracNew> ret;
	bit_t sign = src.data.last();
	for (int i = 0; i < ret.data.size(); i++) {
		if (i < src.data.size())
			_copy(ret.data[i], src.data[i]);
		else
			_copy(ret.data[i], sign);
	}
	return ret;
}

#endif // GLOVEBOX_FIXED32_H
