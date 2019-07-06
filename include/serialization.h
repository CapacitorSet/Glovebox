#ifndef GLOVEBOX_SERIALIZATION_H
#define GLOVEBOX_SERIALIZATION_H

#include <tfhe.h>

void serialize(std::ostream &output, bitspan_t src);
void deserialize(std::istream &input, bitspan_t src);

bit_t make_bit(const std::string &packet);
bitspan_t make_bitspan(int N, const std::string &packet);

std::string serialize(bitspan_t src);

#endif // GLOVEBOX_SERIALIZATION_H
