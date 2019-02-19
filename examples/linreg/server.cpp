#include <assert.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fhe-tools.h>
#include "../networking.h"

#include "../dyad.h"

TFHEServerParams_t default_server_params;

using Q4_4 = Fixed<4, 4>;
Q4_4 *x;

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize, char dataType) {
	puts("New packet.");
	assert(dataType == Q4_4::typeID);
	x = new Q4_4(packet, pktsize);
	auto p = Polynomial<Q4_4>({1.0, 2.0});
	bit_t p_overflow = make_bit();
	auto y = p.evaluate(p_overflow, *x);
	send(stream, &y);
	// send(stream, p_overflow);
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
