#include <assert.h>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>
#include "../networking.h"

#include "../dyad.h"

TFHEClientParams_t default_client_params;

using Q4_4 = Fixed<4, 4>;

Q4_4 *x;

dyad_Stream *s;

void onConnect(dyad_Event *e) {
	send(s, x);
}

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize, char dataType) {
	(void) stream;
	puts("Received:");
	assert(dataType == Q4_4::typeID);
	auto y = Q4_4(packet, pktsize);
	printf("%lf\n", y.toDouble());
}

int main() {
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	default_client_params = makeTFHEClientParams(secret_key);
	fclose(secret_key);

	x = new Q4_4(1.5);

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
	return 0;
}
