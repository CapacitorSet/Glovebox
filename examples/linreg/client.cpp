#include <assert.h>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>
#include <rpc/client.h>

TFHEClientParams_t default_client_params;
weak_params_t default_weak_params;

using Q4_4 = Fixed<4, 4>;

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

	double plaintext_xs[] = {1.0, 1.5};
	puts("Constructing values...");
	Array<Q4_4, 2> xs(false, default_client_params);
	for (int i = 0; i < 2; i++)
		xs.put(plaintext_xs[i], i);

	puts("Connecting to server...");
	rpc::client client("127.0.0.1", 8000);
	puts("Awaiting result...");
	Array<Q4_4, 2> ys =
	    client.call("polyeval", xs.serialize()).as<std::string>();
	for (int i = 0; i < 2; i++) {
		Q4_4 y(default_weak_params);
		ys.get(y, i);
		printf("p(%lf) = %lf\n", plaintext_xs[i], y.toDouble());
	}

	return 0;
}
