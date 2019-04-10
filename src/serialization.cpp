#include <istream>
#include <ostream>
#include <tfhe.h>

#if PLAINTEXT
void serialize(std::ostream &output, bitspan_t src, TFHEClientParams_t) {
	for (const auto bit : src)
		output << *bit.data();
}

void serialize(std::ostream &output, bitspan_t src, TFHEServerParams_t) {
	for (const auto bit : src)
		output << *bit.data();
}

void deserialize(std::istream &input, bitspan_t dst, TFHEClientParams_t p) {
	// The standard approach `input >> *bit.data()` doesn't work for whatever
	// reason
	for (const auto bit : dst) {
		char tmp;
		input >> tmp;
		encrypt(bit, tmp == '1', p);
	}
}

void deserialize(std::istream &input, bitspan_t dst, TFHEServerParams_t p) {
	// The standard approach `input >> *bit.data()` doesn't work for whatever
	// reason
	for (const auto bit : dst) {
		char tmp;
		input >> tmp;
		constant(bit, tmp == '1', p);
	}
}
#else
void serialize(std::ostream &output, bitspan_t src, TFHEClientParams_t p) {
	for (const auto bit : src)
		export_gate_bootstrapping_ciphertext_toStream(output, bit.cptr(),
		                                              p.params);
}

void serialize(std::ostream &output, bitspan_t src, TFHEServerParams_t p) {
	for (const auto bit : src)
		export_gate_bootstrapping_ciphertext_toStream(output, bit.cptr(),
		                                              p.params);
}

void deserialize(std::istream &input, bitspan_t dst, TFHEClientParams_t p) {
	for (const auto bit : dst)
		import_gate_bootstrapping_ciphertext_fromStream(input, bit.cptr(),
		                                                p.params);
}

void deserialize(std::istream &input, bitspan_t dst, TFHEServerParams_t p) {
	for (const auto bit : dst)
		import_gate_bootstrapping_ciphertext_fromStream(input, bit.cptr(),
		                                                p.params);
}
#endif