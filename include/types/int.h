#ifndef FHETOOLS_INT_H
#define FHETOOLS_INT_H

typedef struct {
	char *ptr;
	size_t len;
} ptr_with_length_t;

class Int {
	template <class T> friend class Array;

protected:
	Int(uint8_t _size, bool _isSigned, TFHEServerParams_t _p,
	    bool initialize = true)
			: isSigned(_isSigned), p(_p) {
		if (initialize)
			data = make_bitspan(_size, p);
	}

	bool isSigned;
	bitspan_t data;

public:
	// virtual void writeU8(uint8_t) = 0;

	ptr_with_length_t exportToChar();
	void exportToFile(FILE *out);
	void print(TFHEClientParams_t p);
	void sprint(char *out, TFHEClientParams_t p);
	void free() {
		// it only actually works if the free is commented out.
		// todo: figure out wtf is up
		// free_bitspan(data);
	}

	// todo: rename
	void _writeTo(bitspan_t dst) { _copy(dst, data, p); }
	void _fromBytes(bitspan_t dst) { _copy(data, dst, p); }
	maskable_function_t _m_fromBytes(bit_t mask, bitspan_t dst) {
		return [=] (bit_t mask) -> void {
			for (int i = 0; i < data.size(); i++)
				_mux(data[i], mask, dst[i], data[i], p);
		};
	}

	static const int typeID = INT_TYPE_ID;
	const int size() const { return data.size(); }
	const bool &getSigned() const { return isSigned; }

	Int(char *packet, size_t pktsize, TFHEServerParams_t _p);
	static Int *newU8(TFHEServerParams_t p) { return new Int(8, false, p); }
	static Int *newU8(uint8_t n, TFHEServerParams_t p) {
		auto ret = Int::newU8(p);
		ret->writeU8(n);
		return ret;
	}
	void writeU8(uint8_t);

	void add(Int, Int);
	void mult(Int, Int);
	void copy(Int);

	bit_t isZero() {
		bit_t ret = make_bit(p);
		constant(ret, 1, p);
		for (const auto bit : data)
			_andyn(ret, ret, bit, p);
		return ret;
	}

	bit_t isNonZero() {
		bit_t ret = make_bit(p);
		constant(ret, 0, p);
		for (const auto bit : data)
			_or(ret, ret, bit, p);
		return ret;
	}

private:
	TFHEServerParams_t p;
};

class ClientInt : public Int {
public:
	ClientInt(char *packet, size_t pktsize, TFHEClientParams_t _p)
			: p(_p), Int(packet, pktsize, _p){};
	static ClientInt *newU8(TFHEClientParams_t _p) {
		return new ClientInt(8, false, _p);
	}
	static ClientInt *newU8(uint8_t n, TFHEClientParams_t p) {
		auto ret = ClientInt::newU8(p);
		ret->writeU8(n);
		return ret;
	}
	void writeU8(uint8_t);
	uint8_t toU8();
	ClientInt(uint8_t _size, bool _isSigned, TFHEClientParams_t _p)
			: p(_p), Int(_size, _isSigned, _p) {}

private:
	TFHEClientParams_t p;
};

#endif //FHETOOLS_INT_H
