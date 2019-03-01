#include <fhe-tools.h>

using Q6_2 = Fixed<6, 2>; // Represents weights from 0 to 64 with a precision of 0.25
class Patient {
public:
	// Beware of implicit initializers! They can cause linker errors about
	// default_server_params missing in client code, and outright leaks in
	// server code.
	Int8 resized_height;
	Q6_2 weight;
	Int8 age;
	bit_t isMale;
	bitspan_t data;
	static constexpr int _wordSize = Int8::_wordSize + Q6_2::_wordSize + Int8::_wordSize + 1;
	static constexpr char typeID = 123;

	Patient(TFHEServerParams_t p) : Patient(StructHelper(_wordSize, p), p) {}

	Patient(double _height, double _weight, int8_t _age, bool _isMale, TFHEClientParams_t p)
		: Patient(StructHelper(_wordSize, p), p) {
		int8_t _resized_height = int8_t(_height - 53.0); // Remove offset to make the range fit into an int8
		this->resized_height.encrypt(_resized_height, p);

		double _resized_weight = _weight - 32.0; // Remove offset to make the range fit into an int8
		this->weight.encrypt(_resized_weight, p);

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