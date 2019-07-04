// Computes the 10th Fibonacci number.

#include <assert.h>
#include <cstdio>
#include <tfhe.h>

// Reference: https://tfhe.github.io/tfhe/tuto-alice.html

int main(int argc, char *argv[]) {
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key != nullptr) {
		puts("secret.key exists, exiting.");
		return 1;
	}
	FILE *cloud_key = fopen("cloud.key", "rb");
	if (cloud_key != nullptr) {
		puts("cloud.key exists, exiting.");
		return 1;
	}

	const int minimum_lambda = 110;
	printf("Parameters: minimum_lambda=%d\n", minimum_lambda);
	TFheGateBootstrappingParameterSet *params =
	    new_default_gate_bootstrapping_parameters(minimum_lambda);

	uint32_t seed[] = {314, 1592, 657};
	tfhe_random_generator_setSeed(seed, 3);
	puts("Seed: ");
	for (size_t i = 0; i < sizeof(seed) / sizeof(seed[0]); i++)
		printf("%d ", seed[i]);
	puts("");
	TFheGateBootstrappingSecretKeySet *key =
	    new_random_gate_bootstrapping_secret_keyset(params);

	secret_key = fopen("secret.key", "wb");
	export_tfheGateBootstrappingSecretKeySet_toFile(secret_key, key);
	fclose(secret_key);
	puts("Secret key saved to secret.key.");

	cloud_key = fopen("cloud.key", "wb");
	export_tfheGateBootstrappingCloudKeySet_toFile(cloud_key, &key->cloud);
	fclose(cloud_key);
	puts("Cloud key saved to cloud.key.");

	return 0;
}
