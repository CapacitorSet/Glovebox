#include <fhe-tools.h>

using Q7_1 = Fixed<7, 1>; // Represents weights from 0 to 63 with a precision of 0.5
class Patient {
public:
	// Beware of implicit initializers! They can cause linker errors about
	// default_server_params missing in client code, and outright leaks in
	// server code.
	Int8 resized_height;
	Q7_1 weight;
	Int8 age;
	bit_t isMale;
	bitspan_t data;
	static constexpr int _wordSize = Int8::_wordSize + Q7_1::_wordSize + Int8::_wordSize + 1;
	static constexpr char typeID = 123;

	Patient(TFHEServerParams_t p) : Patient(StructHelper(_wordSize, p), p) {}

	Patient(double _height, double _weight, int8_t _age, bool _isMale, TFHEClientParams_t p)
		: Patient(StructHelper(_wordSize, p), p) {
		this->resized_height.encrypt(_height, p);
		this->weight.encrypt(_weight, p);
		this->age.encrypt(_age, p);
		encrypt(this->isMale, _isMale, p);
	}

private:
	Patient(StructHelper helper, TFHEServerParams_t p)
		: resized_height(helper, p),
		weight(helper, p),
		age(helper, p),
		isMale(helper.make_bit(p)) {
		this->data = helper.finalize();
	}
};