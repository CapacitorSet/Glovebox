#include <glovebox.h>

using Q7_1 = Fixed<7, 1>; // Represents weights from 0 to 63 with a precision of 0.5
class Patient {
  public:
	constexpr static double scaleHeight(double original) {
		return original - 120;
	}
	constexpr static double unscaleHeight(double scaled) {
		return scaled + 120;
	}

	Int8 resized_height;
	Q7_1 weight;
	Int8 age;
	bit_t isMale;
	bitspan_t data;
	static constexpr int _wordSize = Int8::_wordSize + Q7_1::_wordSize + Int8::_wordSize + 1;
	static constexpr char typeID = 123;

	Patient() : Patient(StructHelper(_wordSize)) {}

	Patient(double _height, double _weight, int8_t _age, bool _isMale)
	    : Patient(StructHelper(_wordSize)) {
		this->resized_height.encrypt(scaleHeight(_height));
		this->weight.encrypt(_weight);
		this->age.encrypt(_age);
		encrypt(this->isMale, _isMale);
	}

	double getHeight() {
		return unscaleHeight(this->resized_height.toInt());
	}

  private:
	Patient(StructHelper helper)
	    : resized_height(helper), weight(helper), age(helper), isMale(helper.make_bit()) {
		this->data = helper.finalize();
	}
};