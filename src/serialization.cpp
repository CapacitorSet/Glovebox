#include <istream>
#include <ostream>
#include <serialization.h>
#include <sstream>
#include <tfhe.h>

#if PLAINTEXT
void serialize(std::ostream &output, bitspan_t src, weak_params_t) {
	for (const auto bit : src)
		output << *bit.data();
}

void deserialize(std::istream &input, bitspan_t dst, weak_params_t p) {
	// The standard approach `input >> *bit.data()` doesn't work for whatever
	// reason
	for (const auto bit : dst) {
		char tmp;
		input >> tmp;
		_unsafe_constant(bit, tmp == '1', p);
	}
}

#else
void serialize(std::ostream &output, bitspan_t src, weak_params_t p) {
	for (const auto bit : src)
		export_gate_bootstrapping_ciphertext_toStream(output, bit.cptr(),
		                                              p.params);
}

void deserialize(std::istream &input, bitspan_t dst, weak_params_t p) {
	for (const auto bit : dst)
		import_gate_bootstrapping_ciphertext_fromStream(input, bit.cptr(),
		                                                p.params);
}
#endif

bit_t make_bit(const std::string &packet, weak_params_t p) {
	bit_t ret = make_bit(p);
	std::stringstream ss(packet);
	deserialize(ss, ret, p);
	return ret;
}

bitspan_t make_bitspan(int N, const std::string &packet, weak_params_t p) {
	bitspan_t ret = make_bitspan(N, p);
	std::stringstream ss(packet);
	deserialize(ss, ret, p);
	return ret;
}

std::string exportToString(bitspan_t src, weak_params_t p) {
	std::ostringstream oss;
	serialize(oss, src, p);
	return oss.str();
}