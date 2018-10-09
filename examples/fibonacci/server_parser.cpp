#include "server_parser.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>

// Global, so they can be preserved across onData calls
size_t network_bytes_left;
size_t tfhe_bytes_left;

enum {IDLE, READING_PKT} networkParserState = IDLE;
size_t pktsize;

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define BUFFER_SIZE 100000
#define HEADER_BYTES 5

char buffer[BUFFER_SIZE];
char* dataPtr = buffer;

void onData(dyad_Event *e) {
	network_bytes_left = e->size;
	pkt_read:
	switch (networkParserState) {
		case IDLE: {
			assert(e->size >= HEADER_BYTES);
			char protocol = e->data[0];
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
				onPacket(e->stream, packet, pktsize);

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

void onAccept(dyad_Event *e) {
	puts("Connection received.");
	dyad_addListener(e->remote, DYAD_EVENT_DATA, onData, nullptr);
}

