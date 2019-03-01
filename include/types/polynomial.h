#ifndef FHETOOLS_POLYNOMIAL_H
#define FHETOOLS_POLYNOMIAL_H

#include <vector>
#include "fixed.h"

template <class BaseClass>
class Polynomial {
public:
	explicit Polynomial(std::vector<BaseClass> _factors, TFHEServerParams_t _p = default_server_params)
	: factors(std::move(_factors)), p(_p) {}

	explicit Polynomial(std::vector<double> _factors, TFHEServerParams_t _p = default_server_params)
	: p(_p) {
		for (auto _factor : _factors) {
			factors.push_back(BaseClass(_factor, _p));
		}
	}

	BaseClass evaluate(bit_t overflow, BaseClass x) {
		// Uses Horner's method: p(x) = a_0 + x(a_1 + x(a_2 + ...))
		// todo: skip factors equal to zero if inited from vector<double>
		auto result = BaseClass(p);
		result.copy(factors[factors.size() - 1]);
		constant(overflow, false, p);

		auto old_result = BaseClass(p);
		bit_t overflow_tmp = make_bit(p);
		for (int i = factors.size() - 1; i --> 0; ) {
			constant(overflow_tmp, false, p);
			// Prevents bugs if mul() reads its input after modifying the output
			old_result.copy(result);
			result.mul(overflow_tmp, old_result, x);
			_or(overflow, overflow, overflow_tmp, p);

			constant(overflow_tmp, false, p);
			// Same as above
			old_result.copy(result);
			result.add(overflow_tmp, old_result, factors[i]);
			_or(overflow, overflow, overflow_tmp, p);
		}
		return result;
	}

private:
	std::vector<BaseClass> factors;

	TFHEServerParams_t p;
};

#endif //FHETOOLS_POLYNOMIAL_H
