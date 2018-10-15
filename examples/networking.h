#ifndef FHETOOLS_SERVER_PARSER_H
#define FHETOOLS_SERVER_PARSER_H

#include <cstddef>
#include <dyad.h>
#include <types.h>

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize, char dataType);

void onData(dyad_Event *e);
void onAccept(dyad_Event *e);

void send(dyad_Stream *stream, ClientInt*);
void send(dyad_Stream *stream, ServerInt*);
void sendWithFree(dyad_Stream *stream, char dataType, ptr_with_length_t data);

#endif //FHETOOLS_SERVER_PARSER_H
