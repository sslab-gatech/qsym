#ifndef QSYM_RANGE_H_
#define QSYM_RANGE_H_

// NOTE: some code is from lib/StaticAnalysis/Core/RangeConstraintManager.cpp
#include <cassert>
#include <iostream>
#include <set>
#include <llvm/ADT/APSInt.h>
#include "pin.H"

namespace qsym {

class BitVectorFactory {
public:
  BitVectorFactory(bool is_unsigned, UINT32 bit_width)
    : is_unsigned_(is_unsigned), bit_width_(bit_width) { }

  inline bool isUnsigned() const { return is_unsigned_; }
  inline bool isSigned() const { return !is_unsigned_; }

  llvm::APSInt getValue(llvm::APInt value) const {
    assert(bit_width_ == value.getBitWidth());

    // copy value, but not signdenss
    llvm::APSInt v(value);
    v.setIsUnsigned(is_unsigned_);
    return v;
  }

  llvm::APSInt GetMaxValue() const {
    return llvm::APSInt::getMaxValue(bit_width_, is_unsigned_);
  }

  llvm::APSInt GetMinValue() const {
    return llvm::APSInt::getMinValue(bit_width_, is_unsigned_);
  }

  UINT32 bit_width() const { return bit_width_; }

protected:
  bool is_unsigned_;
  UINT32 bit_width_;
};

class Range : public std::pair<const llvm::APSInt, const llvm::APSInt > {
public:
  Range(const llvm::APSInt from, const llvm::APSInt to)
    : std::pair<const llvm::APSInt, const llvm::APSInt>(from, to) {
    assert(from <= to);
  }

  bool includes(const llvm::APSInt& v) const {
    return first <= v && v <= second;
  }

  const llvm::APSInt &From() const {
    return first;
  }

  const llvm::APSInt &To() const {
    return second;
  }
};


class RangeSet {
public:
  typedef std::set<Range> PrimRangeSet;
  typedef PrimRangeSet::iterator iterator;

  RangeSet(BitVectorFactory BV) : BV_(BV), ranges_() {
    ranges_.insert(Range(BV_.GetMinValue(), BV_.GetMaxValue()));
  }

  RangeSet(BitVectorFactory BV, PrimRangeSet ranges)
    : BV_(BV), ranges_(ranges) {}

  RangeSet(bool is_unsigned, INT32 bit_width) :
    RangeSet(BitVectorFactory(is_unsigned, bit_width)) {}

  iterator begin() const { return ranges_.begin(); }
  iterator end() const { return ranges_.end(); }
  UINT32 size() {
    if (BV_.bit_width() > sizeof(ADDRINT) * CHAR_BIT)
      return INT_MAX / 2; // cannot get value, so return maximum value

    UINT32 res = 0;
    for (iterator i = begin(), e = end(); i != e; ++i)
      res += (i->To() - i->From()).getZExtValue() + 1;
    return res;
  }

  UINT32 num_intervals() {
    return ranges_.size();
  }

protected:
  BitVectorFactory BV_;
  PrimRangeSet ranges_;

  bool isEmpty() const { return ranges_.empty(); }

  void intersectInRange(PrimRangeSet& new_ranges,
                          const llvm::APSInt lower, const llvm::APSInt upper,
                          PrimRangeSet::iterator &i,
                          PrimRangeSet::iterator &e) {
    // There are six cases for each range R in the set:
    //   1. R is entirely before the intersection range.
    //   2. R is entirely after the intersection range.
    //   3. R contains the entire intersection range.
    //   4. R starts before the intersection range and ends in the middle.
    //   5. R starts in the middle of the intersection range and ends after it.
    //   6. R is entirely contained in the intersection range.
    // These correspond to each of the conditions below.
    for (/*i = begin(), e = end()*/; i != e; ++i) {
      if (i->To() < lower) {
        continue;
      }
      if (i->From() > upper) {
        break;
      }

      if (i->includes(lower)) {
        if (i->includes(upper)) {
          new_ranges.insert(Range(lower, upper));
          break;
        } else
          new_ranges.insert(Range(lower, i->To()));
      } else {
        if (i->includes(upper)) {
          new_ranges.insert(Range(i->From(), upper));
          break;
        } else
          new_ranges.insert(*i);
      }
    }
  }

public:
  RangeSet intersect(llvm::APSInt lower, llvm::APSInt upper) {
    // Returns a set containing the values in the receiving set, intersected with
    // the closed range [Lower, Upper]. Unlike the Range type, this range uses
    // modular arithmetic, corresponding to the common treatment of C integer
    // overflow. Thus, if the Lower bound is greater than the Upper bound, the
    // range is taken to wrap around. This is equivalent to taking the
    // intersection with the two ranges [Min, Upper] and [Lower, Max],
    // or, alternatively, /removing/ all integers between Upper and Lower.

    PrimRangeSet new_ranges;
    PrimRangeSet::iterator i = begin(), e = end();

    if (lower <= upper)
      intersectInRange(new_ranges, lower, upper, i, e);
    else {
      // The order of the next two statements is important
      // intersectInRange() does not reset the iteration state for i and e.
      // Therefore, the lower range most be handled first.
      intersectInRange(new_ranges, BV_.GetMinValue(), upper, i, e);
      intersectInRange(new_ranges, lower, BV_.GetMaxValue(), i, e);
    }

    return RangeSet(BV_, new_ranges);
  }

  void print() const {
    print(std::cerr);
  }

  void print(ostream &os) const {
    bool isFirst = true;
    os << "{ ";
    for (iterator i = begin(), e = end(); i != e; ++i) {
      if (isFirst)
        isFirst = false;
      else
        os << ", ";

      os << '[' << i->From().toString(10) << ", " << i->To().toString(10) << ']';
    }
    os << " }";
  }

  RangeSet intersectLT(llvm::APInt value, llvm::APInt adjustment);
  RangeSet intersectLE(llvm::APInt value, llvm::APInt adjustment);
  RangeSet intersectGT(llvm::APInt value, llvm::APInt adjustment);
  RangeSet intersectGE(llvm::APInt value, llvm::APInt adjustment);
  RangeSet intersectEQ(llvm::APInt value, llvm::APInt adjustment);
  RangeSet intersectNE(llvm::APInt value, llvm::APInt adjustment);
};

} // namespace qsym
#endif // QSYM_RANGE_H
