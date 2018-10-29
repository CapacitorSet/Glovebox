#include <assert.h>
#include <cstdio>
#include <tfhe.h>
#include <cstring>
#include "types.h"
#include "../networking.h"

#include "../dyad.h"

ClientArray<ClientInt>* arr;
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
	fclose(secret_key);

	a = ClientInt::newU8(0xde, p);
	b = ClientInt::newU8(0x01, p);
	arr = new ClientArray<ClientInt>(2, 8, p);
	arr->putp(*b, 0);
	arr->put(*a, *b);
	auto arr2 = new ClientArray<ClientInt>(2, 8, p);
	arr2->putp(*b, 0);
	arr2->put(*a, *b);
	printf("Are arrays the same? %d.\n", decrypt(arr->equals(*arr2), p));

	ClientString str("Lorem IPSUM", p);
	ClientString str2("lorem ipsum", p);
	printf("Are strings the same? %d.\n", decrypt(str.equals(str2), p));
	char decrypted[100];
	str.toCStr(decrypted, p);
	printf("Decrypted string: %s\n", decrypted);
	/*
	char data[2] = {(char) 0xde, (char) 0xad};
	arr->put(data, 0, 2);
	*/
	printf("Printing a: ");
	a->print(p);
	printf("\n");
	printf("Printing b: ");
	b->print(p);
	printf("\n");
	_if_else(arr->equals(*arr2), arr->Array::m_get(*a, *b), arr->Array::m_get(*a, *a), makeTFHEServerParams(p));
	printf("Printing a: ");
	a->print(p);
	printf("\n");
	char out[2];
	arr->getp(out, 0);
	arr->getp(out + 1, 1);
	printf("%#02x %#02x\n", out[0], out[1]);

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
	a->free();
	b->free();
	return 0;
}
