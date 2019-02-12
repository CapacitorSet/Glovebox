#include <assert.h>
#include <cstdio>
#include <tfhe.h>
#include <cstring>
#include "types.h"
#include "../networking.h"

#include "../dyad.h"

ClientInt* a;
ClientInt* b;
TFHEClientParams_t default_client_params;
TFHEServerParams_t default_server_params;

dyad_Stream *s;

void onConnect(dyad_Event *e) {
	send(s, a);
	send(s, b);
}

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize, char dataType) {
	puts("New packet.");
	assert(dataType == INT_TYPE_ID);
	auto i = new ClientInt(packet, pktsize);
	printf("Size: %d\n", i->size());
	printf("Value: %d\n", i->toI8());
	i->print();
	putchar('\n');
	delete i;
}

int main(int argc, char *argv[]) {
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	default_client_params = makeTFHEClientParams(secret_key);
	fclose(secret_key);

	a = ClientInt::newI8(1);
	b = ClientInt::newI8(1);

	dyad_init();

	s = dyad_newStream();
	dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, nullptr);
	dyad_addListener(s, DYAD_EVENT_DATA, onData, nullptr);
	dyad_connect(s, "127.0.0.1", 8000);

	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}

	puts("No more connections, closing.");

	dyad_shutdown();
	freeTFHEClientParams(default_client_params);
	delete a;
	delete b;
	return 0;
}
