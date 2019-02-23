#include <assert.h>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>
#include "../networking.h"

#include "../dyad.h"

TFHEClientParams_t default_client_params;

using Q4_4 = Fixed<4, 4>;

Array<Q4_4, 2> *xs;

dyad_Stream *s;

void onConnect(dyad_Event *e) {
	send(s, xs);
}

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize, char dataType) {
	(void) stream;
	puts("Received:");
	assert(dataType == (Array<Q4_4, 2>::typeID));
	auto ys = new Array<Q4_4, 2>(packet, pktsize, default_client_params);
	for (int i = 0; i < 2; i++) {
		auto y = Q4_4(0, default_client_params);
		ys->get(y, i);
		printf("%lf\n", y.toDouble());
	}
}

int main() {
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	default_client_params = makeTFHEClientParams(secret_key);
	fclose(secret_key);

	xs = new Array<Q4_4, 2>(false, default_client_params);
	auto tmp0 = Q4_4(1.0, default_client_params);
	xs->put(tmp0, 0);
	auto tmp1 = Q4_4(1.5, default_client_params);
	xs->put(tmp1, 1);

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
