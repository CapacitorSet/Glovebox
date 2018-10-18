#include <cassert>
#include <cstring>
#include <glob.h>
#include <istream>
#include <sys/stat.h>
#include <types.h>
#include <unistd.h>

// Generic Int code

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

ptr_with_length_t ServerInt::exportToChar() {
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

void ServerInt::parse(char *packet, size_t pktsize,
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
void ServerInt::exportToFile(FILE *out) {
	for (int i = 0; i < size; i++)
		export_gate_bootstrapping_ciphertext_toFile(out, &data[i], p.params);
}

void ServerInt::print(TFHEClientParams_t p) {
	for (int i = size; i-- > 0;)
		printf("%d", decrypt(&data[i], p));
}

void ServerInt::sprint(char *out, TFHEClientParams_t p) {
	for (int i = size; i-- > 0;)
		sprintf(out++, "%d", decrypt(&data[i], p));
}

// ServerInt-specific code

void ServerInt::add(ServerInt a, ServerInt b) {
	assert(size == a.size);
	assert(isSigned == a.isSigned);
	assert(a.size == b.size);
	assert(a.isSigned == b.isSigned);

	// Inputs
	bits_t A, B;
	bits_t CIn = make_bits(1, p);
	constant(CIn, 0, p);

	// Intermediate variables
	bits_t AxorB = make_bits(1, p);
	bits_t AxorBandCIn = make_bits(1, p);
	bits_t AandB = make_bits(1, p);

	// Output variables
	bits_t COut = make_bits(1, p);
	bits_t S;
	for (int i = 0; i < size; i++) {
		A = &a.data[i];
		B = &b.data[i];
		S = &data[i]; // Write to self
		// Todo: remove
		assert(S != A);
		assert(S != B);

		_xor(AxorB, A, B, p);
		_and(AxorBandCIn, AxorB, CIn, p);
		_and(AandB, A, B, p);

		// COut = ((A XOR B) AND CIn) OR (A AND B)
		_or(COut, AxorBandCIn, AandB, p);
		_xor(S, AxorB, CIn, p); // S = (A XOR B) XOR CIn

		// The current COut will be used as CIn.
		_copy(CIn, COut, p);
	}
}

void ServerInt::copy(ServerInt src) {
	assert(size == src.size);
	assert(isSigned == src.isSigned);
	for (int i = 0; i < size; i++) {
		_copy(&data[i], &src.data[i], p);
	}
}

void ServerInt::writeU8(uint8_t val) {
	assert(size == 8);
	assert(!isSigned);
	for (int i = 0; i < 8; i++) {
		constant(&data[i], (val >> i) & 1, p);
	}
}