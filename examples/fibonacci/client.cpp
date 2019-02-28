#include <cassert>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>
#include <rpc/client.h>
#include <rpc/rpc_error.h>
#include <iostream>

TFHEClientParams_t default_client_params;

int main() {
	puts("Initializing TFHE...");
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	default_client_params = makeTFHEClientParams(secret_key);
	fclose(secret_key);

	puts("Constructing values...");
	Int8 a(1, default_client_params);
	Int8 b(1, default_client_params);

	puts("Connecting to server...");
	rpc::client client("127.0.0.1", 8000);
	puts("Awaiting result...");
	std::string result = client.call("fibonacci", 3, a.exportToString(), b.exportToString()).as<std::string>();
	Int8 output(result, default_client_params);
	printf("Result: %i\n", output.toInt());

	return 0;
}
