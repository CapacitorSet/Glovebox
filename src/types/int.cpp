#include <cassert>
#include <cstring>
#include <unistd.h>
#include <sstream>

#include <flow_control.h>
#include <primitives.h>
#include <types/int.h>
#include <serialization.h>

std::string Varint::exportToString() {
	// Todo: header should have >1 byte for size
	char header[1];
	char mysize = size();
	memcpy(header, &mysize, 1);
	printf("Header:\n");
	printf("\tSize: %d\n", header[0]);
	std::ostringstream oss;
	oss << header;
	serialize(oss, data, p);
	return oss.str();
}

Varint::Varint(char *packet, size_t pktsize, TFHEServerParams_t _p) : p(_p) {
	char size;
	memcpy(&size, packet, 1);
	// Skip header
	packet += 1;
	pktsize -= 1;
	printf("Header:\n");
	printf("\tSize: %d\n", size);
	std::stringstream ss;
	ss.write(packet, pktsize);
	data = make_bitspan(size, p);
	deserialize(ss, data, p);
}

void Varint::decrypt(char *dst, TFHEClientParams_t p) {
	for (int i = 0; i < size(); ) {
		char byte = 0;
		for (int j = 0; j < 8 && i < size(); i++, j++)
			byte |= (::decrypt(data[i], p) & 1) << j;
		*dst = byte;
		dst++;
	}
}

int8_t Varint::toI8(TFHEClientParams_t p) {
	assert(size() == 8);
	uint8_t ret = 0;
	for (int i = 0; i < 8; i++)
		ret |= (::decrypt(data[i], p) & 1) << i;
	return ret;
}

void Varint::sprint(char *out, TFHEClientParams_t p) {
	for (int i = size(); i-- > 0;)
		sprintf(out++, "%d", ::decrypt(data[i], p));
}

void Varint::print(TFHEClientParams_t p) {
	for (int i = size(); i-- > 0;)
		printf("%d", ::decrypt(data[i], p));
}

void Varint::copy(Varint src) {
	assert(size() == src.size());
	for (int i = 0; i < size(); i++) {
		_copy(data[i], src.data[i], p);
	}
}