#ifndef GLOVEBOX_STRUCTHELPER_H
#define GLOVEBOX_STRUCTHELPER_H

class StructHelper {
  public:
	StructHelper(uint16_t length, weak_params_t p) {
		this->length = length;
		data = ::make_bitspan(length, p);
	}

	bitspan_t finalize() {
		if (offset != length) {
			fprintf(stderr,
			        "StructHelper: called finalize() with %d bits out of %d. "
			        "Exiting.\n",
			        offset, length);
			abort();
		}
		assert(offset == length);
		return data;
	}

	// The params arg is unused for compatibility with the plain ::make_bit.
	// It will come in useful if ::make_bit is also refactored to some kind
	// of allocator.
	bit_t make_bit(weak_params_t) {
		assert_fits(1);
		return data[offset++];
	}

	bitspan_t make_bitspan(uint8_t length, weak_params_t) {
		assert_fits(length);
		auto ret = data.subspan(offset, length);
		offset += length;
		return ret;
	}

	template <uint8_t size> fixed_bitspan_t<size> make_bitspan(weak_params_t) {
		assert_fits(size);
		auto ret = data.subspan<size>(offset);
		offset += size;
		return fixed_bitspan_t<size>(ret);
	}

  private:
	uint16_t offset = 0;
	uint16_t length;
	// This is private; users must access it via finalize() to enforce size
	// checks
	bitspan_t data;

	void assert_fits(uint8_t size) {
		if ((length - offset) < size) {
			fprintf(stderr,
			        "StructHelper: tried to allocate %d bits, only %d left. "
			        "Exiting.\n",
			        size, (length - offset));
			abort();
		}
	}
};
#endif