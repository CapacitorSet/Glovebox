#include <assert.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <tfhe.h>
#include <glob.h>
#include "types.h"

#include "dyad.h"

#define BUFFER_SIZE 100000
#define HEADER_BYTES 5

TFHEServerParams_t params;

// Global, so they can be preserved across onData calls
size_t network_bytes_left;
size_t tfhe_bytes_left;

enum {IDLE, READING_PKT} networkParserState = IDLE;
size_t pktsize;
char buffer[BUFFER_SIZE];
char* dataPtr = buffer;

#define min(a, b) (((a) < (b)) ? (a) : (b))

static void onPacket(char *packet, size_t pktsize) {
	puts("New FHET packet.");
	ServerInt i(packet, pktsize, params);
	printf("Received Int with size %d.\n", i.size);
}

static void onData(dyad_Event *e) {
	network_bytes_left = e->size;
pkt_read:
	switch (networkParserState) {
		case IDLE: {
			assert(e->size >= HEADER_BYTES);
			char protocol = e->data[0];
			printf("Protocol: %d\n", protocol);
			assert(protocol == PROTOCOL_VERSION);
			memcpy(&pktsize, e->data + 1, 4);
			tfhe_bytes_left = pktsize;
			network_bytes_left -= HEADER_BYTES;
			networkParserState = READING_PKT;
			// Fallthrough to reading code
			e->data += HEADER_BYTES;
		}
		case READING_PKT: {
			// Read a whole packet, if possible
			size_t readSize = min(pktsize, min(network_bytes_left, tfhe_bytes_left));
			network_bytes_left -= readSize;
			tfhe_bytes_left -= readSize;
			// Assert that the write will be within bounds
			assert(dataPtr + readSize <= buffer + BUFFER_SIZE);
			memcpy(dataPtr, e->data, readSize);
			dataPtr += readSize;
			if (tfhe_bytes_left == 0) {
				char* packet = static_cast<char *>(malloc(pktsize));
				memcpy(packet, buffer, pktsize);
				onPacket(packet, pktsize);

				free(packet);
				dataPtr = buffer;
				networkParserState = IDLE;
			}
			if (network_bytes_left != 0) {
				e->data += readSize;
				goto pkt_read;
			}
			break;
		}
		default:
			printf("Unknown state %d!\n", networkParserState);
			exit(-1);
	}
	assert(network_bytes_left == 0);
}

static void onAccept(dyad_Event *e) {
	puts("Connection received.");
	dyad_addListener(e->remote, DYAD_EVENT_DATA, onData, nullptr);
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

	dyad_Stream *serv = dyad_newStream();
	dyad_addListener(serv, DYAD_EVENT_ACCEPT, onAccept, nullptr);
	dyad_listen(serv, 8000);
	puts("Ready to accept connections.");

	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}

	puts("Exiting gracefully.");
	dyad_shutdown();
	return 0;
}