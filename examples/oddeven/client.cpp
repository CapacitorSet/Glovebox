#include <assert.h>
#include <cstdio>
#include <tfhe.h>
#include <cstring>
#include "types.h"
#include "../networking.h"

#include "dyad.h"

ClientInt* a;
ClientInt* b;
TFHEClientParams_t p;

dyad_Stream *s;

void onConnect(dyad_Event *e) {
	send(s, a);
	send(s, b);
}

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize, char dataType) {
	puts("New packet.");
	assert(dataType == INT_TYPE_ID);
	auto i = new ClientInt(packet, pktsize, p);
	printf("Size: %d\n", i->getSize());
	printf("Value: %d\n", i->toU8());
	i->print(p);
	putchar('\n');
	delete i;
}

int main(int argc, char *argv[]) {
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	p = makeTFHEClientParams(secret_key);
	fclose(secret_key);

	a = ClientInt::newU8(1, p);
	b = ClientInt::newU8(1, p);

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
	freeTFHEClientParams(p);
	delete a;
	delete b;
	return 0;
}
