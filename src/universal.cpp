#include <cassert>
#include <cstring>
#include <glob.h>
#include <istream>
#include <sys/stat.h>
#include <types.h>
#include <unistd.h>

FILE *charptr_to_file(char *src, size_t len) {
	char *filename = std::tmpnam(nullptr);
	printf("charptr_to_file: %s\n", filename);
	FILE *wptr = fopen(filename, "wb");
	assert(wptr);
	fwrite(src, len, 1, wptr);
	fclose(wptr);

	FILE *rptr = fopen(filename, "rb");
	assert(rptr);
	return rptr;
}

ptr_with_length_t Int::exportToChar() {
	// Todo: header should have >1 byte for size
	char header[2];
	memcpy(header, &isSigned, 1);
	memcpy(header + 1, &size, 1);
	printf("Header:\n");
	printf("\tSigned: %d\n", header[0]);
	printf("\tSize: %d\n", header[1]);
	char *filename = std::tmpnam(nullptr);
	printf("exportToChar: %s\n", filename);
	FILE *wptr = fopen(filename, "wb");
	assert(wptr);
	exportToFile(wptr);
	fclose(wptr);

	// https://stackoverflow.com/a/238609
	struct stat st;
	stat(filename, &st);
	long _size = st.st_size + sizeof(header);
	assert(_size <= SIZE_MAX); // So it can be cast to size_t safely
	auto size = static_cast<size_t>(_size);
	FILE *rptr = fopen(filename, "rb");
	assert(rptr);
	char *out = static_cast<char *>(malloc(size));
	memcpy(out, header, sizeof(header));
	fread(out + sizeof(header), 1, size, rptr);
	fclose(rptr);
	// unlink(filename);
	return ptr_with_length_t{out, size};
}

void Int::parse(char *packet, size_t pktsize,
                const TFheGateBootstrappingParameterSet *params) {
	memcpy(&isSigned, packet, 1);
	memcpy(&size, packet + 1, 1);
	// Skip header
	packet += 2;
	pktsize -= 2;
	printf("Header:\n");
	printf("\tSigned: %d\n", isSigned);
	printf("\tSize: %d\n", size);
	FILE *f = charptr_to_file(packet, pktsize);
	if (f == nullptr) {
		printf("errno: %d\n", errno);
		printf("Error opening file: %s\n", strerror(errno));
		exit(-1);
	}
	assert(f);
	data = new_gate_bootstrapping_ciphertext_array(size, params);
	for (int i = 0; i < size; i++)
		import_gate_bootstrapping_ciphertext_fromFile(f, &data[i], params);
}

// todo: document that it doesn't export a header
void Int::exportToFile(FILE *out) {
	for (int i = 0; i < size; i++)
		export_gate_bootstrapping_ciphertext_toFile(out, &data[i], params);
}

void Int::print(TFHEClientParams_t p) {
	for (int i = size; i-- > 0;)
		printf("%d", decrypt(&data[i], p));
}

void Int::sprint(char *out, TFHEClientParams_t p) {
	for (int i = size; i-- > 0;)
		sprintf(out++, "%d", decrypt(&data[i], p));
}
