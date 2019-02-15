#include <cassert>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>
#include "../networking.h"

#include "../dyad.h"

TFHEClientParams_t default_client_params;
TFHEServerParams_t default_server_params;

Varint* first;
Varint* second;

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize, char dataType) {
	puts("New packet.");
	assert(dataType == INT_TYPE_ID);
	auto tmp = new Varint(packet, pktsize);
	if (first == nullptr) {
		first = tmp;
	} else if (second == nullptr) {
		second = tmp;

		auto x = Varint::newI8();
		for (int i = 0; i < 3; i++) {
			printf("Iteration %d\n", i);
			x->add(*first, *second);
			// todo: document that this is buggy af because it will result in ptrs being reused
			// first = second;
			// second = x;
			first->copy(*second);
			second->copy(*x);
		}
		send(stream, x);
	}
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
	return 0;
}
