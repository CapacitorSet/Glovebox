#include <cassert>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>
#include <iostream>
#include <rpc/client.h>
#include <rpc/rpc_error.h>

TFHEClientParams_t default_client_params;
weak_params_t default_weak_params;

int main() {
	puts("Initializing TFHE...");
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	default_weak_params = default_client_params =
	    makeTFHEClientParams(secret_key);
	fclose(secret_key);

	puts("Constructing values...");
	// Automatic encryption. Fancy, huh?
	Int8 a = 1;
	Int8 b = 1;

	puts("Connecting to server...");
	rpc::client client("127.0.0.1", 8000);
	puts("Awaiting result...");
	// Watch this: automatic deserialization.
	Int8 output = client.call("fibonacci", 3, a.serialize(), b.serialize())
	                  .as<std::string>();
	printf("Result: %i\n", output.toInt());

	return 0;
}
