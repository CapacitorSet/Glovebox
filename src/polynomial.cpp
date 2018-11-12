#include "types.h"

Fixed32 KnownPolynomial::evaluate(Fixed32 x) {
	// powers[1] = x
	// powers[2] = x ** 2
	// etc
	// todo: memoize in a decent way
	Fixed32 *powersOfX = reinterpret_cast<Fixed32*>(malloc(sizeof(Fixed32) * factors.size()));
	switch (factors.size()) {
		case 1:
			return factors[0];
		case 2:
			return factors[0].plus(factors[1].times(x));
		default:
			Fixed32 ret = factors[0].plus(factors[1].times(x));
			powersOfX[0] = x;
			for (int i = 2; i < factors.size(); i++) {
				powersOfX[i - 1] = x.times(powersOfX[i - 2]);
				ret = ret.plus(factors[i].times(powersOfX[i - 1]));
			}
			free(powersOfX);
			return ret;
	}
}