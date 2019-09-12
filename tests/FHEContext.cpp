#include "gtest/gtest.h"
#include <tfhe.h>

class FHEContext : public ::testing::Test {
  protected:
	void SetUp() override {
		key = read_client_key("secret.key");
		if (key == nullptr) {
			puts("secret.key not found: run ./keygen first.");
			ASSERT_NE(key, nullptr);
		}
		params = ClientParams(key);
		serverParams = ServerParams(params);
	}

	void TearDown() override { free_client_key(key); }

	ClientKey key;
	ClientParams params;
	ServerParams serverParams;
};