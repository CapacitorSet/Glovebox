#include "gtest/gtest.h"
#include "../include/tfhe.h"

class FHEContext : public ::testing::Test {
protected:
	void SetUp() override {
		FILE *secret_key = fopen("secret.key", "rb");
		if (secret_key == nullptr) {
			puts("secret.key not found: run ./keygen first.");
			ASSERT_NE(secret_key, nullptr);
		}
		clientParams = makeTFHEClientParams(secret_key);
		serverParams = (TFHEServerParams_t) clientParams;
		fclose(secret_key);
	}

	void TearDown() override {
		freeTFHEClientParams(clientParams);
	}

	TFHEClientParams_t clientParams;
	TFHEServerParams_t serverParams;
};