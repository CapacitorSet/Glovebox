#ifndef FHETOOLS_SERIALIZATION_H
#define FHETOOLS_SERIALIZATION_H

#include <tfhe.h>

void serialize(std::ostream &output, bitspan_t src, TFHEClientParams_t p);
void serialize(std::ostream &output, bitspan_t src, TFHEServerParams_t p);

void deserialize(std::istream &input, bitspan_t src, TFHEClientParams_t p);
void deserialize(std::istream &input, bitspan_t src, TFHEServerParams_t p);

bit_t make_bit(const std::string &packet,
               TFHEServerParams_t p = default_server_params);
bit_t make_bit(const std::string &packet,
               TFHEClientParams_t p = default_client_params);

bitspan_t make_bitspan(int N, const std::string &packet,
                       TFHEServerParams_t p = default_server_params);
bitspan_t make_bitspan(int N, const std::string &packet,
                       TFHEClientParams_t p = default_client_params);

std::string exportToString(bitspan_t src, TFHEServerParams_t);
std::string exportToString(bitspan_t src, TFHEClientParams_t);

#endif // FHETOOLS_SERIALIZATION_H
