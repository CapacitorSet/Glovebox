#include <assert.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <tfhe.h>
#include "types.h"
#include "server_parser.h"

#include "dyad.h"

TFHEServerParams_t params;

ServerInt* first;
ServerInt* second;

void send(dyad_Stream *stream, ptr_with_length_t data) {
	char header[5] = {PROTOCOL_VERSION};
	memcpy(header + 1, &data.len, 4);
	dyad_write(stream, header, 5);
	printf("Sending %zu bytes.\n", data.len);
	dyad_write(stream, data.ptr, data.len);
}

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize) {
	puts("New packet.");
	auto tmp = new ServerInt(packet, pktsize, params);
	if (first == nullptr) {
		puts("Assigning to first");
		first = tmp;
	} else if (second == nullptr) {
		puts("Assigning to second");
		second = tmp;

		auto x = ServerInt::newU8(params);
		for (int i = 0; i < 3; i++) {
			printf("Sum %d\n", i);
			x->add(*first, *second);
			break;
			first = second;
			second = x;
		}
		puts("Send");
		send(stream, x->exportToChar());
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
