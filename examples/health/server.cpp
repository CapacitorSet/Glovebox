#include <cassert>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>
#include <rpc/server.h>

#include "patient.h"

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

	Array<Patient, 10> database;

	rpc::server srv(8000);

	srv.bind("uploadDatabase", [&](std::string _db) {
		puts("Receiving database...");
		database = _db;
		puts("Database received.");
	});

	srv.run();

	return 0;
}
