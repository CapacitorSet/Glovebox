#include <assert.h>
#include <cstdio>
#include <tfhe.h>
#include <cstring>
#include "types.h"
#include "../networking.h"

#include "../dyad.h"

TFHEClientParams_t p;

dyad_Stream *s;

void onConnect(dyad_Event *e) {
	/*
	send(s, a);
	send(s, b);
	*/
}

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize, char dataType) {
	puts("New packet.");
	assert(dataType == INT_TYPE_ID);
	auto i = new ClientInt(packet, pktsize, p);
	printf("Size: %d\n", i->size());
	printf("Value: %d\n", i->toU8());
	i->print(p);
	putchar('\n');
	delete i;
}

// Todo: implement
int main(int argc, char *argv[]) {
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	p = makeTFHEClientParams(secret_key);
	auto _p = makeTFHEServerParams(p);
	fclose(secret_key);

	Int *x = Int::newU8(15, _p);
	Int *y = Int::newU8(15, _p);
	Int *res = Int::newU8(_p);
	res->mult(*x, *y);
	printf("Result of multiplication: ");
	res->print(p);
	printf("\n");

	ClientFixed32 a = ClientFixed32(2.0, p);
	printf("Current value: a=%f\n", a.times(ClientFixed32(100.5, p)).toFloat(p));

	KnownPolynomial pol = KnownPolynomial(std::vector<float>({1.0f, 2.0f, 1.0f}), makeTFHEServerParams(p));
	printf("p(x) = 1.0 in x=a: %f\n", pol.evaluate(a).toFloat(p));

	/*
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
	 */
	freeTFHEClientParams(p);
	return 0;
}
