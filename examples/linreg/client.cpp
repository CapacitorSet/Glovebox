#include <assert.h>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>
#include <rpc/client.h>

TFHEClientParams_t default_client_params;

using Q4_4 = Fixed<4, 4>;

int main() {
	puts("Initializing TFHE...");
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	default_client_params = makeTFHEClientParams(secret_key);
	fclose(secret_key);

	double plaintext_xs[] = {1.0, 1.5};
	puts("Constructing values...");
	Array<Q4_4, 2> xs(false, default_client_params);
	for (int i = 0; i < 2; i++)
		xs.put(Q4_4(plaintext_xs[i], default_client_params), i);

	puts("Connecting to server...");
	rpc::client client("127.0.0.1", 8000);
	puts("Awaiting result...");
	std::string result = client.call("polyeval", xs.exportToString()).as<std::string>();
	Array<Q4_4, 2> ys(result.c_str(), result.size(), default_client_params);
	for (int i = 0; i < 2; i++) {
		Q4_4 y(0, default_client_params);
		ys.get(y, i);
		printf("p(%lf) = %lf\n", plaintext_xs[i], y.toDouble());
	}

	return 0;
}
