#include <assert.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <tfhe.h>
#include "types.h"
#include "../networking.h"

#include "dyad.h"

TFHEServerParams_t params;

ServerInt* first;
ServerInt* second;

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize) {
	puts("New packet.");
	auto tmp = new ServerInt(packet, pktsize, params);
	if (first == nullptr) {
		first = tmp;
	} else if (second == nullptr) {
		second = tmp;

		auto x = ServerInt::newU8(params);
		for (int i = 0; i < 3; i++) {
			printf("Iteration %d\n", i);
			x->add(*first, *second);
			// todo: document that this is buggy af because it will result in ptrs being reused
			// first = second;
			// second = x;
			first->copy(*second);
			second->copy(*x);
		}
		sendWithFree(stream, x->exportToChar());
	}
}

int main() {
	FILE *cloud_key = fopen("cloud.key", "rb");
	if (cloud_key == nullptr) {
		puts("cloud.key not found: run ./keygen first.");
		return 1;
	}
	puts("Initializing TFHE...");
	params = makeTFHEServerParams(cloud_key);
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
