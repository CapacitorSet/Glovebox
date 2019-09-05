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

#ifndef BITVEC_DECL_H
#define BITVEC_DECL_H

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

} // namespace gb

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BITVEC_DECL_H
