#include "FHEContext.cpp"
#include "gtest/gtest.h"
#include <rapidcheck/gtest.h>

using Serialization = FHEContext;

TEST_F(Serialization, Serialize) {
	::rc::detail::checkGTest([=](uint32_t input) {
		std::stringstream ss;
		bitspan_t datum = make_bitspan(32), out_datum = make_bitspan(32);
		for (int i = 0; i < 32; i++)
			encrypt(datum[i], (input >> i) & 1);
		serialize(ss, datum);
		deserialize(ss, out_datum);
		for (int i = 0; i < 32; i++)
			RC_ASSERT(decrypt(datum[i]) == decrypt(out_datum[i]));
	});
}