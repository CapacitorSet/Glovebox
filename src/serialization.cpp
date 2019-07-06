#include <istream>
#include <ostream>
#include <serialization.h>
#include <sstream>
#include <tfhe.h>

#if PLAINTEXT
void serialize(std::ostream &output, bitspan_t src) {
	for (const auto bit : src)
		output << *bit.data();
}

void deserialize(std::istream &input, bitspan_t dst) {
	// The standard approach `input >> *bit.data()` doesn't work for whatever
	// reason
	for (const auto bit : dst) {
		char tmp;
		input >> tmp;
		_unsafe_constant(bit, tmp == '1');
	}
}

#else
void serialize(std::ostream &output, bitspan_t src) {
	for (const auto bit : src)
		export_gate_bootstrapping_ciphertext_toStream(output, bit.cptr(), weak_params.params);
}

void deserialize(std::istream &input, bitspan_t dst) {
	for (const auto bit : dst)
		import_gate_bootstrapping_ciphertext_fromStream(input, bit.cptr(), weak_params.params);
}
#endif

bit_t make_bit(const std::string &packet) {
	bit_t ret = make_bit();
	std::stringstream ss(packet);
	deserialize(ss, ret);
	return ret;
}

bitspan_t make_bitspan(int N, const std::string &packet) {
	bitspan_t ret = make_bitspan(N);
	std::stringstream ss(packet);
	deserialize(ss, ret);
	return ret;
}

std::string serialize(bitspan_t src) {
	std::ostringstream oss;
	serialize(oss, src);
	return oss.str();
}