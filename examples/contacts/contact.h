#ifndef FHETOOLS_PHONE_NUMBER_H
#define FHETOOLS_PHONE_NUMBER_H

#include <types/int.h>
using PhoneNumber = Int<35>; // 2**34 covers more than 10 decimal digits
using Contact = struct {
	std::string name;
	int64_t phoneNumber;
};

#endif // FHETOOLS_PHONE_NUMBER_H
