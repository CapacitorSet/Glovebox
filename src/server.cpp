#include <cassert>
#include <cstring>
#include <glob.h>
#include <istream>
#include <sys/stat.h>
#include <types.h>
#include <unistd.h>
#include <types/int.h>

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

ptr_with_length_t Int::exportToChar() {
	// Todo: header should have >1 byte for size
	char header[1];
	char mysize = size();
	memcpy(header, &mysize, 1);
	printf("Header:\n");
	printf("\tSize: %d\n", header[0]);
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

Int::Int(char *packet, size_t pktsize, TFHEServerParams_t _p) : p(_p) {
	char size;
	memcpy(&size, packet, 1);
	// Skip header
	packet += 1;
	pktsize -= 1;
	printf("Header:\n");
	printf("\tSize: %d\n", size);
	FILE *f = charptr_to_file(packet, pktsize);
	if (f == nullptr) {
		printf("errno: %d\n", errno);
		printf("Error opening file: %s\n", strerror(errno));
		exit(-1);
	}
	assert(f);
	data = make_bitspan(size, p);
#if PLAINTEXT
	assert("TODO: implement" == 0);
#else
	for (int i = 0; i < size; i++)
		import_gate_bootstrapping_ciphertext_fromFile(f, &data.at(i), p.params);
#endif
}

// todo: document that it doesn't export a header
void Int::exportToFile(FILE *out) {
#if PLAINTEXT
	assert("TODO: implement" == 0);
	(void)out;
#else
	for (auto bit : data)
		export_gate_bootstrapping_ciphertext_toFile(out, bit.data(), p.params);
#endif
}

void Int::decrypt(char *dst, TFHEClientParams_t p) {
	for (int i = 0; i < size(); ) {
		char byte = 0;
		for (int j = 0; j < 8 && i < size(); i++, j++)
			byte |= (::decrypt(data[i], p) & 1) << j;
		*dst = byte;
		dst++;
	}
}

int8_t Int::toI8(TFHEClientParams_t p) {
	assert(size() == 8);
	uint8_t ret = 0;
	for (int i = 0; i < 8; i++)
		ret |= (::decrypt(data[i], p) & 1) << i;
	return ret;
}

void Int::sprint(char *out, TFHEClientParams_t p) {
	for (int i = size(); i-- > 0;)
		sprintf(out++, "%d", ::decrypt(data[i], p));
}

void Int::print(TFHEClientParams_t p) {
	for (int i = size(); i-- > 0;)
		printf("%d", ::decrypt(data[i], p));
}

// ServerInt-specific code

void Int::add(Int a, Int b) {
	assert(size() == a.size());
	assert(size() == b.size());

	::add(data, a.data, b.data, p);
}

void Int::mult(Int a, Int b) {
	assert(size() == a.size());
	assert(size() == b.size());

	auto dummy = make_bitspan(2 * size(), p);
	::mult(dummy, a.data, b.data, p);
	_copy(data, dummy.subspan(0, 8), p);
}

void Int::copy(Int src) {
	assert(size() == src.size());
	for (int i = 0; i < size(); i++) {
		_copy(data[i], src.data[i], p);
	}
}

static uint8_t highest_bit_set(uint64_t val) {
	uint8_t ret = 0;
	while (val >>= 1) ret++;
	return ret;
}

void Int::write(int64_t val) {
	assert(highest_bit_set(labs(val)) <= size()); // Check that the value fits
	for (int i = 0; i < size(); i++)
		constant(data[i], (val >> i) & 1, p);
}