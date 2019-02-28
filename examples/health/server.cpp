#include <cassert>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>

#include "patient.h"
#include "../networking.h"
#include "../dyad.h"

TFHEServerParams_t default_server_params;

Array<Patient, 10>* database;

void onPacket(dyad_Stream*, char *packet, size_t pktsize, char dataType) {
	puts("New packet.");
	static bool was_database_initialized = false;
	if (!was_database_initialized) {
		assert(dataType == ARRAY_TYPE_ID);
		database = new Array<Patient, 10>(packet, pktsize);
		was_database_initialized = true;
		puts("Database initialized.");
		return;
	}
	// If we got here, we have a query.
	// ...
}

int main() {
	FILE *cloud_key = fopen("cloud.key", "rb");
	if (cloud_key == nullptr) {
		puts("cloud.key not found: run ./keygen first.");
		return 1;
	}
	puts("Initializing TFHE...");
	default_server_params = makeTFHEServerParams(cloud_key);
	fclose(cloud_key);

	dyad_init();

	auto stream = dyad_newStream();
	dyad_addListener(stream, DYAD_EVENT_ACCEPT, onAccept, nullptr);
	dyad_listen(stream, 8000);
	printf("Listening: %s:%d\n", dyad_getAddress(stream), dyad_getPort(stream));

	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}

	puts("Exiting gracefully.");
	dyad_shutdown();
	delete database;
	return 0;
}
