#include <cassert>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>
#include "../networking.h"

#include "../dyad.h"

Int8 *a;
Int8 *b;
TFHEClientParams_t default_client_params;
TFHEServerParams_t default_server_params;

dyad_Stream *s;

void onConnect(dyad_Event *e) {
	puts("Sending...");
	send(s, a);
	send(s, b);
}

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize, char dataType) {
	puts("Received:");
	assert(dataType == Int8::typeID);
	Int8 output(packet, pktsize);
	printf("%d\n", output.toI8());
}

int main() {
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	default_client_params = makeTFHEClientParams(secret_key);
	fclose(secret_key);

	a = new Int8(1);
	b = new Int8(1);

	dyad_init();

	s = dyad_newStream();
	dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, nullptr);
	dyad_addListener(s, DYAD_EVENT_DATA, onData, nullptr);
	puts("Connecting...");
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
