#ifndef GLOVEBOX_SERIALIZATION_H
#define GLOVEBOX_SERIALIZATION_H

#include <tfhe.h>

void serialize(std::ostream &output, bitvec_t src);
void deserialize(std::istream &input, bitvec_t src);

bit_t make_bit(const std::string &packet);
bitvec_t make_bitvec(int N, const std::string &packet);

std::string serialize(bitvec_t src);

#endif // GLOVEBOX_SERIALIZATION_H
