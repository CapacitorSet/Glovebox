#include <istream>
#include <ostream>
#include <tfhe.h>

#if PLAINTEXT
void serialize(std::ostream &output, bitspan_t src, TFHEClientParams_t p) {
	(void) p;
	for (const auto bit : src)
		output << *bit.data();
}

void serialize(std::ostream &output, bitspan_t src, TFHEServerParams_t p) {
	(void) p;
	for (const auto bit : src)
		output << *bit.data();
}

void deserialize(std::istream &input, bitspan_t dst, TFHEClientParams_t p) {
	(void) p;
	for (const auto bit : dst)
		input >> *bit.data();
}

void deserialize(std::istream &input, bitspan_t dst, TFHEServerParams_t p) {
	(void) p;
	for (const auto bit : dst)
		input >> *bit.data();
}
#else
void serialize(std::ostream &output, bitspan_t src, TFHEClientParams_t p) {
	for (const auto bit : src)
		export_gate_bootstrapping_ciphertext_toStream(output, bit.data(), p.params);
}

void serialize(std::ostream &output, bitspan_t src, TFHEServerParams_t p) {
	for (const auto bit : src)
		export_gate_bootstrapping_ciphertext_toStream(output, bit.data(), p.params);
}

void deserialize(std::istream &input, bitspan_t dst, TFHEClientParams_t p) {
	for (const auto bit : dst)
		import_gate_bootstrapping_ciphertext_fromStream(input, bit.data(), p.params);
}

void deserialize(std::istream &input, bitspan_t dst, TFHEServerParams_t p) {
	for (const auto bit : dst)
		import_gate_bootstrapping_ciphertext_fromStream(input, bit.data(), p.params);
}
#endif