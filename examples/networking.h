#ifndef FHETOOLS_SERVER_PARSER_H
#define FHETOOLS_SERVER_PARSER_H

#include "dyad.h"
#include <fhe-tools.h>

void onPacket(dyad_Stream *stream, char *packet, size_t pktsize, char dataType);

void onData(dyad_Event *e);
void onAccept(dyad_Event *e);

template<class T>
void send(dyad_Stream *stream, T* obj) {
	sendRaw(stream, T::typeID, obj->exportToString());
};
void sendRaw(dyad_Stream *s, char dataType, std::string data);

#endif //FHETOOLS_SERVER_PARSER_H
