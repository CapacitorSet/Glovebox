#ifndef FHETOOLS_POLYNOMIAL_H
#define FHETOOLS_POLYNOMIAL_H

class KnownPolynomial {
public:
	KnownPolynomial(std::vector<Fixed32> _factors, TFHEServerParams_t _p) : factors(std::move(_factors)), p(_p) {
	}
	KnownPolynomial(std::vector<float> _factors, TFHEServerParams_t _p) : p(_p) {
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
