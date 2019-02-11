#ifndef FHETOOLS_SERIALIZATION_H
#define FHETOOLS_SERIALIZATION_H

#include <istream>
#include <ostream>
#include <tfhe.h>

void serialize(std::ostream &output, bitspan_t src, TFHEClientParams_t p);
void serialize(std::ostream &output, bitspan_t src, TFHEServerParams_t p);

void deserialize(std::istream &input, bitspan_t src, TFHEClientParams_t p);
void deserialize(std::istream &input, bitspan_t src, TFHEServerParams_t p);

#endif //FHETOOLS_SERIALIZATION_H
