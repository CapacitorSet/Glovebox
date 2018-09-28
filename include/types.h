#ifndef FHE_TOOLS_TYPES_H
#define FHE_TOOLS_TYPES_H

#include "tfhe.h"
#include <cstdint>
#include <glob.h>

bits_t bits_merge(bits_t, bits_t);

class Array;

typedef struct {
	char* ptr;
	size_t len;
} ptr_with_length_t;

class Int {
public:
	virtual void writeU8(uint8_t) = 0;
	virtual void exportToFile(FILE *out) = 0;
	ptr_with_length_t exportToChar();

	uint8_t size; // todo: move back to protected when i'm done debugging
protected:
	bool isSigned;
	bits_t data;
};

class ClientInt : public Int {
public:
	static ClientInt* newU8(TFHEClientParams_t p) {
		return new ClientInt(8, false, p);
	}
	static ClientInt* newU8(uint8_t n, TFHEClientParams_t p) {
		auto ret = ClientInt::newU8(p);
		ret->writeU8(n);
		return ret;
	}
	void writeU8(uint8_t) final override;
	void exportToFile(FILE *out) final override;
	ClientInt(uint8_t size, bool isSigned, TFHEClientParams_t p);
private:
	TFHEClientParams_t p;
};

class ServerInt : public Int {
	// Required to access .size
	friend class Array;

  public:
	ServerInt(char *packet, size_t pktsize, TFHEServerParams_t p);
	static ServerInt newU8(TFHEServerParams_t p) {
		return {8, false, p};
	}
	static ServerInt newU8(uint8_t n, TFHEServerParams_t p) {
		auto ret = ServerInt::newU8(p);
		ret.writeU8(n);
		return ret;
	}
	void writeU8(uint8_t) final override;
	void exportToFile(FILE *out) final override;
	ServerInt(uint8_t size, bool isSigned, TFHEServerParams_t p);

	void add(ServerInt, ServerInt);
	void copy(ServerInt);

	static bits_t isZero(ServerInt n) {
		bits_t ret = make_bits(1, n.p);
		constant(ret, 1, n.p);
		for (int i = 0; i < n.size; i++)
			_andyn(ret, ret, &n.data[i], n.p);
		return ret;
	}

	static bits_t isNonZero(ServerInt n) {
		bits_t ret = make_bits(1, n.p);
		constant(ret, 1, n.p);
		for (int i = 0; i < n.size; i++)
			_and(ret, ret, &n.data[i], n.p);
		_not(ret, ret, n.p);
		return ret;
	}

	bits_t bits() { return data; }

#if DEBUG
	void print();
#endif
  private:
	TFHEServerParams_t p;
};

class Array {
  public:
	Array(uint64_t length, uint16_t wordSize, TFHEServerParams_t p);
	void getN_thInt(ServerInt ret, const bits_t address, uint8_t bitsInAddress);
	void putN_thInt(ServerInt src, const bits_t address, uint8_t bitsInAddress);

  private:
	uint64_t length;
	uint16_t wordSize;
	bits_t data;
	TFHEServerParams_t p;

	static void getN_thBit(bits_t ret, uint8_t N, uint8_t wordsize,
	                       const bits_t address, uint8_t bitsInAddress,
	                       const bits_t staticOffset, size_t dynamicOffset,
	                       TFHEServerParams_t p);
	static void putN_thBit(bits_t src, uint8_t N, uint8_t wordsize,
	                       const bits_t address, uint8_t bitsInAddress,
	                       const bits_t staticOffset, size_t dynamicOffset,
	                       bits_t mask, TFHEServerParams_t p);
};

#endif // FHE_TOOLS_TYPES_H
