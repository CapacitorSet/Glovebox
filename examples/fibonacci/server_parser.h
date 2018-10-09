#ifndef FHETOOLS_SERVER_PARSER_H
#define FHETOOLS_SERVER_PARSER_H

#include <cstddef>
#include <dyad.h>

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize);

void onData(dyad_Event *e);
void onAccept(dyad_Event *e);

#endif //FHETOOLS_SERVER_PARSER_H
