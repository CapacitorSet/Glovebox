#ifndef FHE_TOOLS_TYPES_H
#define FHE_TOOLS_TYPES_H

#include "tfhe.h"
#include <cstdint>
#include <glob.h>

// The enum is not actually used, it just ensures that type IDs do not overlap
enum {
	UNINITIALIZED_TYPE_ID = 0,
	INT_TYPE_ID,
	ARRAY_TYPE_ID,
};

template <class T>
class Array;

bits_t bits_merge(bits_t, bits_t);

typedef struct {
	char* ptr;
	size_t len;
} ptr_with_length_t;

class Int {
	template <class T>
	friend class Array;
public:
	virtual void writeU8(uint8_t) = 0;

	ptr_with_length_t exportToChar();
	void exportToFile(FILE *out);
	void print(TFHEClientParams_t p);
	void sprint(char *out, TFHEClientParams_t p);
	void free() {
		// it only actually works if the free is commented out.
		// todo: figure out wtf is up

		// printf("Freeing %d items.\n", size);
		// free_LweSample_array(size, data);
	}

	const int& getSize() const { return size; }
	const bool& getSigned() const { return isSigned; }
protected:
	Int(uint8_t _size, bool _isSigned, const TFheGateBootstrappingParameterSet* _params, bool initialize = true)
		: size(_size), isSigned(_isSigned), params(_params) {
		if (initialize)
			data = new_gate_bootstrapping_ciphertext_array(_size, _params);
	}
	explicit Int(const TFheGateBootstrappingParameterSet* _params) : params(_params) {}

	void parse(char *packet, size_t pktsize, const TFheGateBootstrappingParameterSet *params);

	uint8_t size;
	bool isSigned;
	bits_t data;
	const TFheGateBootstrappingParameterSet* params;
};

class ClientInt : public Int {
public:
	ClientInt(char *packet, size_t pktsize, TFHEClientParams_t _p) : p(_p), Int(_p.params) {
		parse(packet, pktsize, _p.params);
	};
	static ClientInt* newU8(TFHEClientParams_t _p) {
		return new ClientInt(8, false, _p);
	}
	static ClientInt* newU8(uint8_t n, TFHEClientParams_t p) {
		auto ret = ClientInt::newU8(p);
		ret->writeU8(n);
		return ret;
	}
	void writeU8(uint8_t) final override;
	uint8_t toU8();
	ClientInt(uint8_t _size, bool _isSigned, TFHEClientParams_t _p) : p(_p), Int(_size, _isSigned, _p.params) {}
	void operator delete(void *_ptr) {
		auto ptr = static_cast<ClientInt*>(_ptr);
		ptr->free();
	}
private:
	TFHEClientParams_t p;
};

class ServerInt : public Int {
  public:
	ServerInt(char *packet, size_t pktsize, TFHEServerParams_t _p) : p(_p), Int(_p.params) {
		parse(packet, pktsize, _p.params);
	};
	static ServerInt* newU8(TFHEServerParams_t p) {
		return new ServerInt(8, false, p);
	}
	static ServerInt* newU8(uint8_t n, TFHEServerParams_t p) {
		auto ret = ServerInt::newU8(p);
		ret->writeU8(n);
		return ret;
	}
	void writeU8(uint8_t) final override;
	ServerInt(uint8_t _size, bool _isSigned, TFHEServerParams_t _p) : p(_p), Int(_size, _isSigned, _p.params) {}

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
  private:
	TFHEServerParams_t p;
};

template <class T>
class Array {
  public:
	Array(uint64_t _length, uint16_t _wordSize, TFHEServerParams_t p) : length(_length), wordSize(_wordSize) {
		data = make_bits(_length * _wordSize, p);
	};
	void getN_th(T ret, const bits_t address, uint8_t bitsInAddress);
	void putN_th(T src, const bits_t address, uint8_t bitsInAddress);

  private:
	uint64_t length;
	uint16_t wordSize;
	bits_t data;
	TFHEServerParams_t p;
	const TFheGateBootstrappingParameterSet* params;

	static void getN_thBit(bits_t ret, uint8_t N, uint8_t wordsize,
	                       const bits_t address, uint8_t bitsInAddress,
	                       const bits_t staticOffset, size_t dynamicOffset,
	                       TFHEServerParams_t p);
	static void putN_thBit(bits_t src, uint8_t N, uint8_t wordsize,
	                       const bits_t address, uint8_t bitsInAddress,
	                       const bits_t staticOffset, size_t dynamicOffset,
	                       bits_t mask, TFHEServerParams_t p);
};

template <typename T>
class ClientArray : Array<T> {

};

#endif // FHE_TOOLS_TYPES_H
