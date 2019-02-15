#ifndef FHETOOLS_POLYNOMIAL_H
#define FHETOOLS_POLYNOMIAL_H

#include <vector>
#include "fixed32.h"

class KnownPolynomial {
public:
	explicit KnownPolynomial(std::vector<Fixed32> _factors, TFHEServerParams_t _p = default_server_params)
	: factors(std::move(_factors)), p(_p) {}

	explicit KnownPolynomial(std::vector<float> _factors, TFHEServerParams_t _p = default_server_params)
	: p(_p) {
		for (auto _factor : _factors) {
			factors.push_back(Fixed32(_factor, _p));
		}
	}

	Fixed32 evaluate(Fixed32 x);

private:
	std::vector<Fixed32> factors;

	TFHEServerParams_t p;
};

#endif //FHETOOLS_POLYNOMIAL_H
