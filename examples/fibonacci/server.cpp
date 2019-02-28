#include <cassert>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>
#include <rpc/server.h>

TFHEServerParams_t default_server_params;

int main() {
	puts("Initializing TFHE...");
	FILE *cloud_key = fopen("cloud.key", "rb");
	if (cloud_key == nullptr) {
		puts("cloud.key not found: run ./keygen first.");
		return 1;
	}
	default_server_params = makeTFHEServerParams(cloud_key);
	fclose(cloud_key);

	rpc::server srv(8000);

	srv.bind("fibonacci", [](int times, std::string a, std::string b) {
		puts("Received request.");
		Int8 first(a.c_str(), a.size());
		Int8 second(b.c_str(), a.size());
		Int8 ret(0);
		for (int i = 0; i < times; i++) {
			printf("Iteration %d\n", i);
			ret.add(first, second);
			// todo: document that this is buggy af because it will result in ptrs being reused
			// first = second;
			// second = x;
			first.copy(second);
			second.copy(ret);
		}
		return ret.exportToString();
	});

	srv.run();

	return 0;
}
