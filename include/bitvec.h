///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef GSL_SPAN_H
#define GSL_SPAN_H

#include <gsl/gsl_assert> // for Expects
#include <gsl/gsl_util>   // for narrow_cast, narrow

using gsl::narrow_cast;

#include <cstddef>  // for ptrdiff_t, size_t, nullptr_t
#include <iterator> // for reverse_iterator, distance, random_access_...
#include <memory>

#ifdef _MSC_VER
#pragma warning(push)

// turn off some warnings that are noisy about our Expects statements
#pragma warning(disable : 4127) // conditional expression is constant
#pragma warning(disable : 4702) // unreachable code

// Turn MSVC /analyze rules that generate too much noise. TODO: fix in the tool.
#pragma warning(                                                               \
    disable : 26495) // uninitalized member when constructor calls constructor
#pragma warning(                                                               \
    disable : 26446) // parser bug does not allow attributes on some templates

#endif // _MSC_VER

namespace gb {

// shared_ptr without atomic overhead
template <typename T>
using shared_ptr_unsynchronized = std::__shared_ptr<T, __gnu_cxx::_S_single>;

// [views.constants], constants
const std::ptrdiff_t dynamic_extent = -1;

template <std::ptrdiff_t Extent = dynamic_extent> class bitvec;

// implementation details
namespace details {
template <class Span, bool IsConst> class span_iterator {
	using element_type_ = typename Span::element_type;

  public:
#ifdef _MSC_VER
	// Tell Microsoft standard library that span_iterators are checked.
	using _Unchecked_type = typename Span::pointer;
#endif

	using iterator_category = std::random_access_iterator_tag;
	using value_type = std::remove_cv_t<element_type_>;
	using difference_type = typename Span::index_type;

	using pointer = shared_ptr_unsynchronized<
	    std::conditional_t<IsConst, const element_type_, element_type_>>;

	span_iterator() = default;

	span_iterator(const Span *bitvec, typename Span::index_type idx) noexcept
	    : span_(bitvec), index_(idx) {}

	friend span_iterator<Span, true>;
	template <bool B, std::enable_if_t<!B && IsConst> * = nullptr>
	span_iterator(const span_iterator<Span, B> &other) noexcept
	    : span_iterator(other.span_, other.index_) {}

	std::conditional_t<IsConst, const bitvec<1>, bitvec<1>> operator*() const {
		Expects(index_ != span_->size());
		return std::conditional_t<IsConst, const bitvec<1>, bitvec<1>>(
		    (*span_)[index_]);
	}

	pointer operator->() const {
		Expects(index_ != span_->size());
		return span_->data() + index_;
	}

	span_iterator &operator++() {
		Expects(0 <= index_ && index_ != span_->size());
		++index_;
		return *this;
	}

	span_iterator &operator--() {
		Expects(index_ != 0 && index_ <= span_->size());
		--index_;
		return *this;
	}

	pointer operator[](difference_type n) const { return *(*this + n); }

	friend bool operator==(span_iterator lhs, span_iterator rhs) noexcept {
		return lhs.span_ == rhs.span_ && lhs.index_ == rhs.index_;
	}

	friend bool operator!=(span_iterator lhs, span_iterator rhs) noexcept {
		return !(lhs == rhs);
	}

	friend bool operator<(span_iterator lhs, span_iterator rhs) noexcept {
		return lhs.index_ < rhs.index_;
	}

#ifdef _MSC_VER
	// MSVC++ iterator debugging support; allows STL algorithms in 15.8+
	// to unwrap span_iterator to a pointer type after a range check in STL
	// algorithm calls
	friend void _Verify_range(
	    span_iterator lhs,
	    span_iterator rhs) noexcept {  // test that [lhs, rhs) forms a valid
		                               // range inside an STL algorithm
		Expects(lhs.span_ == rhs.span_ // range spans have to match
		        && lhs.index_ <= rhs.index_); // range must not be transposed
	}

	void _Verify_offset(const difference_type n) const
	    noexcept { // test that the iterator *this + n is a valid range in an
		           // STL
		// algorithm call
		Expects((index_ + n) >= 0 && (index_ + n) <= span_->size());
	}

	GSL_SUPPRESS(bounds .1)               // NO-FORMAT: attribute
	pointer _Unwrapped() const noexcept { // after seeking *this to a high water
		                                  // mark, or using one of the
		// _Verify_xxx functions above, unwrap this span_iterator to a raw
		// pointer
		return span_->data() + index_;
	}

	// Tell the STL that span_iterator should not be unwrapped if it can't
	// validate in advance, even in release / optimized builds:
	static const bool _Unwrap_when_unverified = false;
	GSL_SUPPRESS(con .3) // NO-FORMAT: attribute // TODO: false positive
	void _Seek_to(const pointer p) noexcept { // adjust the position of *this to
		                                      // previously verified location p
		// after _Unwrapped
		index_ = p - span_->data();
	}
#endif

  protected:
	const Span *span_ = nullptr;
	std::ptrdiff_t index_ = 0;
};

// Used for empty base class optimization
template <std::ptrdiff_t Ext> class extent_type {
  public:
	using index_type = std::ptrdiff_t;

	static_assert(Ext >= 0, "A fixed-size bitvec must be >= 0 in size.");

	extent_type() noexcept {}

	template <index_type Other> extent_type(extent_type<Other> ext) {
		static_assert(Other == Ext || Other == dynamic_extent,
		              "Mismatch between fixed-size extent and size of "
		              "initializing data.");
		Expects(ext.size() == Ext);
	}

	extent_type(index_type size) { Expects(size == Ext); }

	index_type size() const noexcept { return Ext; }
};

template <> class extent_type<dynamic_extent> {
  public:
	using index_type = std::ptrdiff_t;

	template <index_type Other>
	explicit extent_type(extent_type<Other> ext) : size_(ext.size()) {}

	explicit extent_type(index_type size) : size_(size) { Expects(size >= 0); }

	index_type size() const noexcept { return size_; }

  private:
	index_type size_;
};

template <std::ptrdiff_t Extent, std::ptrdiff_t Offset, std::ptrdiff_t Count>
struct calculate_subspan_type {
	using type =
	    bitvec<Count != dynamic_extent
	             ? Count
	             : (Extent != dynamic_extent ? Extent - Offset : Extent)>;
};
} // namespace details

template <std::ptrdiff_t Extent> class bitvec {
  public:
	// constants and types
	using element_type = unsafe_bit_t;
	using value_type = std::remove_cv_t<element_type>;
	using index_type = std::ptrdiff_t;
	using pointer = shared_ptr_unsynchronized<element_type>;

	using iterator = details::span_iterator<bitvec<Extent>, false>;
	using const_iterator = details::span_iterator<bitvec<Extent>, true>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	using size_type = index_type;

	// [bitvec.cons], bitvec constructors, copy, assignment, and destructor
	template <bool Dependent = false,
	          // "Dependent" is needed to make "std::enable_if_t<Dependent ||
	          // Extent <= 0>" SFINAE, since "std::enable_if_t<Extent <= 0>" is
	          // ill-formed when Extent is greater than 0.
	          class = std::enable_if_t<(Dependent || Extent <= 0)>>
	bitvec() noexcept : storage_(nullptr, details::extent_type<0>()) {}

	bitvec(pointer ptr, index_type count) : storage_(ptr, count) {}

	bitvec(const bitvec &other) noexcept = default;

	template <std::ptrdiff_t OtherExtent,
	          class = std::enable_if_t<OtherExtent == Extent ||
	                                   Extent == dynamic_extent>>
	bitvec(const bitvec<OtherExtent> &other)
	    : storage_(other.data(),
	               details::extent_type<OtherExtent>(other.size())) {}

	~bitvec() noexcept = default;
	bitvec &operator=(const bitvec &other) noexcept = default;

	// [bitvec.sub], bitvec subviews
	template <std::ptrdiff_t Count> bitvec<Count> first() const {
		Expects(Count >= 0 && Count <= size());
		return {data(), Count};
	}

	template <std::ptrdiff_t Count = 1>
	GSL_SUPPRESS(bounds .1) // NO-FORMAT: attribute
	bitvec<Count> last() const {
		Expects(Count >= 0 && size() - Count >= 0);
		return {data_plus(size() - Count), Count};
	}

	template <std::ptrdiff_t Offset, std::ptrdiff_t Count = dynamic_extent>
	GSL_SUPPRESS(bounds .1) // NO-FORMAT: attribute
	auto subspan() const ->
	    typename details::calculate_subspan_type<Extent, Offset, Count>::type {
		Expects((Offset >= 0 && size() - Offset >= 0) &&
		        (Count == dynamic_extent ||
		         (Count >= 0 && Offset + Count <= size())));

		return {data_plus(Offset),
		        Count == dynamic_extent ? size() - Offset : Count};
	}

	template <std::ptrdiff_t Count>
	bitvec<Count> subspan(std::ptrdiff_t offset) const {
		Expects((offset >= 0 && size() - offset >= 0) &&
		        (Count == dynamic_extent ||
		         (Count >= 0 && offset + Count <= size())));

		return {data_plus(offset),
		        Count == dynamic_extent ? size() - offset : Count};
	}

	bitvec<dynamic_extent> first(index_type count) const {
		Expects(count >= 0 && count <= size());
		return {data(), count};
	}

	bitvec<dynamic_extent> last(index_type count) const {
		return make_subspan(size() - count, dynamic_extent,
		                    subspan_selector<Extent>{});
	}

	bitvec<dynamic_extent> subspan(index_type offset,
	                             index_type count = dynamic_extent) const {
		return make_subspan(offset, count, subspan_selector<Extent>{});
	}

	// [bitvec.obs], bitvec observers
	index_type size() const noexcept { return storage_.size(); }
	index_type size_bytes() const noexcept {
		return size() * narrow_cast<index_type>(sizeof(element_type));
	}
	bool empty() const noexcept { return size() == 0; }

	bitvec<1> operator[](index_type idx) const { return {data_plus(idx), 1}; }

	pointer at(index_type idx) const {
		Expects(CheckRange(idx, idx < storage_.size()));
		return data()[idx];
	}
	pointer operator()(index_type idx) const { return this->operator[](idx); }
	pointer data() const noexcept { return storage_.data(); }
	element_type *cptr() const noexcept { return storage_.data().get(); }

	// [bitvec.iter], bitvec iterator support
	iterator begin() const noexcept { return {this, 0}; }
	iterator end() const noexcept { return {this, size()}; }

	const_iterator cbegin() const noexcept { return {this, 0}; }
	const_iterator cend() const noexcept { return {this, size()}; }

	reverse_iterator rbegin() const noexcept { return reverse_iterator{end()}; }
	reverse_iterator rend() const noexcept { return reverse_iterator{begin()}; }

	const_reverse_iterator crbegin() const noexcept {
		return const_reverse_iterator{cend()};
	}
	const_reverse_iterator crend() const noexcept {
		return const_reverse_iterator{cbegin()};
	}

#ifdef _MSC_VER
	// Tell MSVC how to unwrap spans in range-based-for
	pointer _Unchecked_begin() const noexcept { return data(); }
	pointer _Unchecked_end() const noexcept {
		GSL_SUPPRESS(bounds .1) // NO-FORMAT: attribute
		return data() + size();
	}
#endif // _MSC_VER

  private:
	static constexpr bool CheckRange(index_type idx, index_type size) {
		// Optimization:
		//
		// idx >= 0 && idx < size
		// =>
		// static_cast<size_t>(idx) < static_cast<size_t>(size)
		//
		// because size >=0 by bitvec construction, and negative idx will
		// wrap around to a value always greater than size when casted.

		// check if we have enough space to wrap around
		if (sizeof(index_type) <= sizeof(size_t)) {
			return narrow_cast<size_t>(idx) < narrow_cast<size_t>(size);
		} else {
			return idx >= 0 && idx < size;
		}
	}

	// Needed to remove unnecessary null check in subspans
	struct KnownNotNull {
		pointer p;
	};

	// this implementation detail class lets us take advantage of the
	// empty base class optimization to pay for only storage of a single
	// pointer in the case of fixed-size spans
	template <class ExtentType> class storage_type : public ExtentType {
	  public:
		// KnownNotNull parameter is needed to remove unnecessary null check
		// in subspans and constructors from arrays
		template <class OtherExtentType>
		storage_type(KnownNotNull data, OtherExtentType ext)
		    : ExtentType(ext), data_(data.p) {
			Expects(ExtentType::size() >= 0);
		}

		template <class OtherExtentType>
		storage_type(pointer data, OtherExtentType ext)
		    : ExtentType(ext), data_(data) {
			Expects(ExtentType::size() >= 0);
			Expects(data || ExtentType::size() == 0);
		}

		pointer data() const noexcept { return data_; }

	  private:
		pointer data_;
	};

	storage_type<details::extent_type<Extent>> storage_;

	// The rest is needed to remove unnecessary null check
	// in subspans and constructors from arrays
	bitvec(KnownNotNull ptr, index_type count) : storage_(ptr, count) {}

	template <std::ptrdiff_t CallerExtent> class subspan_selector {};

	template <std::ptrdiff_t CallerExtent>
	bitvec<dynamic_extent> make_subspan(index_type offset, index_type count,
	                                  subspan_selector<CallerExtent>) const {
		const bitvec<dynamic_extent> tmp(*this);
		return tmp.subspan(offset, count);
	}

	GSL_SUPPRESS(bounds .1) // NO-FORMAT: attribute
	bitvec<dynamic_extent> make_subspan(index_type offset, index_type count,
	                                  subspan_selector<dynamic_extent>) const {
		Expects(offset >= 0 && size() - offset >= 0);

		pointer p = data_plus(offset);

		if (count == dynamic_extent) {
			return {KnownNotNull{p}, size() - offset};
		}

		Expects(count >= 0 && size() - offset >= count);
		return {KnownNotNull{p}, count};
	}

  private:
	// Implements `data() + offset` taking care of ownership.
	pointer data_plus(index_type offset) const {
		Expects(offset >= 0 && size() - offset >= 0);

		// Uses same ref counter as data()
		return pointer(data(), data().get() + offset);
	}
};

} // namespace gb

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // GSL_SPAN_H
