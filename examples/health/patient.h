#include <fhe-tools.h>

using Q6_2 = Fixed<6, 2>; // Represents weights from 0 to 64 with a precision of 0.25
class Patient {
public:
	Int8 resized_height;
	Q6_2 weight;
	Int8 age;
	bit_t isMale;
	bitspan_t data;
	static constexpr int _wordSize = Int8::_wordSize + Q6_2::_wordSize + Int8::_wordSize + 1;
	static constexpr char typeID = 123;

	Patient(double _height, double _weight, int8_t _age, bool _isMale, TFHEClientParams_t p)
			: isMale(make_bit(p)) {
		StructHelper helper(_wordSize, p);

		int8_t _resized_height = int8_t(_height - 53.0); // Remove offset to make the range fit into an int8
		this->resized_height = Int8(_resized_height, helper, p);

		double _resized_weight = _weight - 32.0; // Remove offset to make the range fit into an int8
		this->weight = Q6_2(_resized_weight, helper, p);

		Int8 age(_age, helper, p);
		this->isMale = helper.make_bit(p);
		encrypt(isMale, _isMale, p);

		this->data = helper.finalize();
	}
};