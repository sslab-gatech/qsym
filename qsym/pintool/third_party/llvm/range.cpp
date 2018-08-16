#include "range.h"

namespace qsym {
  RangeSet RangeSet::intersectLT(llvm::APInt value, llvm::APInt adjustment) {
    llvm::APSInt adj = BV_.getValue(adjustment);
    llvm::APSInt val = BV_.getValue(value);
    llvm::APSInt min = BV_.GetMinValue();

    llvm::APSInt lower = min - adj;
    llvm::APSInt upper = val - adj;
    --upper;

    return intersect(lower, upper);
  }

  RangeSet RangeSet::intersectLE(llvm::APInt value, llvm::APInt adjustment) {
    llvm::APSInt adj = BV_.getValue(adjustment);
    llvm::APSInt val = BV_.getValue(value);
    llvm::APSInt min = BV_.GetMinValue();

    llvm::APSInt lower = min - adj;
    llvm::APSInt upper = val - adj;
    return intersect(lower, upper);
  }

  RangeSet RangeSet::intersectGT(llvm::APInt value, llvm::APInt adjustment) {
    llvm::APSInt adj = BV_.getValue(adjustment);
    llvm::APSInt val = BV_.getValue(value);
    llvm::APSInt max = BV_.GetMaxValue();

    llvm::APSInt lower = val - adj;
    ++lower;
    llvm::APSInt upper = max - adj;
    return intersect(lower, upper);
  }

  RangeSet RangeSet::intersectGE(llvm::APInt value, llvm::APInt adjustment) {
    llvm::APSInt adj = BV_.getValue(adjustment);
    llvm::APSInt val = BV_.getValue(value);
    llvm::APSInt max = BV_.GetMaxValue();

    llvm::APSInt lower = val - adj;
    llvm::APSInt upper = max - adj;
    return intersect(lower, upper);
  }

  RangeSet RangeSet::intersectEQ(llvm::APInt value, llvm::APInt adjustment) {
    llvm::APSInt adj = BV_.getValue(adjustment);
    llvm::APSInt val = BV_.getValue(value);

    llvm::APSInt lower = val - adj;
    llvm::APSInt upper = val - adj;
    return intersect(lower, upper);
  }

  RangeSet RangeSet::intersectNE(llvm::APInt value, llvm::APInt adjustment) {
    llvm::APSInt adj = BV_.getValue(adjustment);
    llvm::APSInt val = BV_.getValue(value);

    llvm::APSInt lower = val - adj;
    ++lower;
    llvm::APSInt upper = val - adj;
    --upper;
    return intersect(lower, upper);
  }
} // namespace qsym
