#include "gtest/gtest.h"
#include <fhe-tools.h>

class FHEContext : public ::testing::Test {
  protected:
	void SetUp() override {
		FILE *secret_key = fopen("secret.key", "rb");
		if (secret_key == nullptr) {
			puts("secret.key not found: run ./keygen first.");
			ASSERT_NE(secret_key, nullptr);
		}
		params = makeTFHEClientParams(secret_key);
		serverParams = TFHEServerParams_t(params);
		fclose(secret_key);
	}

	void TearDown() override { freeTFHEClientParams(params); }

	TFHEClientParams_t params;
	TFHEServerParams_t serverParams;
};