#ifndef GLOVEBOX_SERIALIZATION_H
#define GLOVEBOX_SERIALIZATION_H

#include <tfhe.h>

void serialize(std::ostream &output, bitspan_t src, WeakParams p);
void deserialize(std::istream &input, bitspan_t src, WeakParams p);

bit_t make_bit(const std::string &packet, WeakParams p = default_weak_params);
bitspan_t make_bitspan(int N, const std::string &packet,
                       WeakParams p = default_weak_params);

std::string serialize(bitspan_t src, WeakParams);

#endif // GLOVEBOX_SERIALIZATION_H
