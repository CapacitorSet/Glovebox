#ifndef FHETOOLS_INT_H
#define FHETOOLS_INT_H

typedef struct {
	char *ptr;
	size_t len;
} ptr_with_length_t;

class Int {
	template <class T> friend class Array;

protected:
	bitspan_t data;

public:
	explicit Int(uint8_t _size, TFHEServerParams_t _p = default_server_params) : p(_p) {
		data = make_bitspan(_size, p);
	}

	// virtual void writeU8(uint8_t) = 0;

	void decrypt(char *dst, TFHEClientParams_t p = default_client_params);
	int8_t toI8(TFHEClientParams_t p = default_client_params);
	ptr_with_length_t exportToChar();
	void exportToFile(FILE *out);
	void print(TFHEClientParams_t p = default_client_params);
	void sprint(char *out, TFHEClientParams_t p = default_client_params);
	void free() {
		// it only actually works if the free is commented out.
		// todo: figure out wtf is up
		// free_bitspan(data);
	}

	// todo: rename
	void _writeTo(bitspan_t dst) { _copy(dst, data, p); }
	void _fromBytes(bitspan_t dst) { _copy(data, dst, p); }
	maskable_function_t _m_fromBytes(bitspan_t dst) {
		return [=] (bit_t mask) -> void {
			for (int i = 0; i < data.size(); i++)
				_mux(data[i], mask, dst[i], data[i], p);
		};
	}

	static const int typeID = INT_TYPE_ID;
	int size() const { return data.size(); }

	Int(char *packet, size_t pktsize, TFHEServerParams_t _p = default_server_params);
	static Int *newI8(TFHEServerParams_t p = default_server_params) { return new Int(8, p); }
	static Int *newI8(int8_t n, TFHEServerParams_t p = default_server_params) {
		auto ret = Int::newI8(p);
		ret->write(n);
		return ret;
	}
	void write(int64_t);

	void add(Int, Int);
	void mult(Int, Int);
	void copy(Int);

private:
	TFHEServerParams_t p;
};

class ClientInt : public Int {
public:
	ClientInt(char *packet, size_t pktsize, TFHEClientParams_t _p = default_client_params)
			: Int(packet, pktsize, _p), p(_p) {};
	static ClientInt *newI8(TFHEClientParams_t _p = default_client_params) {
		return new ClientInt(8, _p);
	}
	static ClientInt *newI8(int8_t n, TFHEClientParams_t p = default_client_params) {
		auto ret = ClientInt::newI8(p);
		ret->write(n);
		return ret;
	}
	void write(int64_t);
	explicit ClientInt(uint8_t _size, TFHEClientParams_t _p = default_client_params)
			: Int(_size, _p), p(_p) {}

private:
	TFHEClientParams_t p;
};

#endif //FHETOOLS_INT_H
