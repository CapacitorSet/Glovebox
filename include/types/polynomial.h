#ifndef GLOVEBOX_POLYNOMIAL_H
#define GLOVEBOX_POLYNOMIAL_H

#include "fixed.h"
#include <vector>

template <class BaseClass> class Polynomial {
  public:
	explicit Polynomial(std::vector<BaseClass> _factors) : factors(std::move(_factors)) {}

	explicit Polynomial(std::vector<double> _factors) {
		for (auto _factor : _factors) {
			factors.push_back(BaseClass(_factor));
		}
	}

	BaseClass evaluate(bit_t overflow, BaseClass x) const {
		// Uses Horner's method: p(x) = a_0 + x(a_1 + x(a_2 + ...))
		// todo: skip factors equal to zero if inited from vector<double>
		auto result = BaseClass();
		result.copy(factors[factors.size() - 1]);
		overflow = false;

		BaseClass old_result;
		bit_t overflow_tmp = make_bit();
		for (int i = factors.size() - 1; i-- > 0;) {
			overflow_tmp = false;
			// Prevents bugs if mul() reads its input after modifying the output
			old_result.copy(result);
			result.mul(overflow_tmp, old_result, x);
			overflow |= overflow_tmp;

			overflow_tmp = false;
			// Same as above
			old_result.copy(result);
			result.add(overflow_tmp, old_result, factors[i]);
			overflow |= overflow_tmp;
		}
		return result;
	}

  private:
	std::vector<BaseClass> factors;
};

#endif // GLOVEBOX_POLYNOMIAL_H
