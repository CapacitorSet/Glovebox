#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "networking.h"

#define PROTOCOL_VERSION 1

// Global, so they can be preserved across onData calls
size_t network_bytes_left;
size_t tfhe_bytes_left;

enum {IDLE, READING_PKT} networkParserState = IDLE;
size_t pktSize;
char dataType;

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define BUFFER_SIZE 100000
#define HEADER_BYTES 6
/* Header:
 *
 *   1 byte: protocol version
 *   4 bytes: packet size
 *   1 byte: data type
 */

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
			memcpy(&pktSize, e->data + 1, 4);
			memcpy(&dataType, e->data + 5, 1);
			tfhe_bytes_left = pktSize;
			network_bytes_left -= HEADER_BYTES;
			networkParserState = READING_PKT;
			// Fallthrough to reading code
			e->data += HEADER_BYTES;
		}
		case READING_PKT: {
			// Read a whole packet, if possible
			size_t readSize = min(pktSize, min(network_bytes_left, tfhe_bytes_left));
			network_bytes_left -= readSize;
			tfhe_bytes_left -= readSize;
			// Assert that the write will be within bounds
			assert(dataPtr + readSize <= buffer + BUFFER_SIZE);
			memcpy(dataPtr, e->data, readSize);
			dataPtr += readSize;
			if (tfhe_bytes_left == 0) {
				char* packet = static_cast<char *>(malloc(pktSize));
				memcpy(packet, buffer, pktSize);
				onPacket(e->stream, packet, pktSize, dataType);

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

void sendWithFree(dyad_Stream *s, char dataType, ptr_with_length_t data) {
	char header[HEADER_BYTES] = {PROTOCOL_VERSION};
	memcpy(header + 1, &data.len, 4);
	memcpy(header + 5, &dataType, 1);
	dyad_write(s, header, HEADER_BYTES);
	dyad_write(s, data.ptr, data.len);
	free(data.ptr);
}

void send(dyad_Stream *s, ClientInt *i) {
	sendWithFree(s, INT_TYPE_ID, i->exportToChar());
}
void send(dyad_Stream *s, ServerInt *i) {
	sendWithFree(s, INT_TYPE_ID, i->exportToChar());
}