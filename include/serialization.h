#ifndef FHETOOLS_SERIALIZATION_H
#define FHETOOLS_SERIALIZATION_H

#include <tfhe.h>

void serialize(std::ostream &output, bitspan_t src, weak_params_t p);
void deserialize(std::istream &input, bitspan_t src, weak_params_t p);

bit_t make_bit(const std::string &packet,
               weak_params_t p = default_weak_params);
bitspan_t make_bitspan(int N, const std::string &packet,
                       weak_params_t p = default_weak_params);

std::string serialize(bitspan_t src, weak_params_t);

#endif // FHETOOLS_SERIALIZATION_H
