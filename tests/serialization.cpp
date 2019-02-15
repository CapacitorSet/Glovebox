#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>
#include "FHEContext.cpp"

using Serialization = FHEContext;

TEST_F(Serialization, Serialize) {
	::rc::detail::checkGTest([=](uint32_t input) {
		std::stringstream ss;
		bitspan_t datum = make_bitspan(32, clientParams),
			out_datum = make_bitspan(32, clientParams);
		for (int i = 0; i < 32; i++)
			encrypt(datum[i], (input >> i) & 1, clientParams);
		serialize(ss, datum, clientParams);
		deserialize(ss, out_datum, clientParams);
		for (int i = 0; i < 32; i++)
			RC_ASSERT(decrypt(datum[i], clientParams) == decrypt(out_datum[i], clientParams));
	});
}