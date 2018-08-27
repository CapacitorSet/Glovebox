// Computes the 10th Fibonacci number.

#include <cstdio>
#include <tfhe.h>
#include "types.h"

#if !DEBUG
#error The example must be compiled in debug mode to read the output.
#endif

int main(int argc, char *argv[]) {
	FILE *cloud_key = fopen("cloud.key","rb");
	TFHEParams p = makeTFHEParams(cloud_key);
#if !IS_PLAINTEXT
	FILE* secret_key = fopen("secret.key","rb");
	TFheGateBootstrappingSecretKeySet* key = new_tfheGateBootstrappingSecretKeySet_fromFile(secret_key);
	p.secret_key = key;
	fclose(secret_key);
#endif
	fclose(cloud_key);
	Int a = Int::newU8(1, p);
	Int b = Int::newU8(1, p);
	Int c = Int::newU8(p);
	for (int i = 0; i < 10; i++) {
		c.add(a, b);

		a.copy(b);
		b.copy(c);
	}
	c.print();
    return 0;
}
