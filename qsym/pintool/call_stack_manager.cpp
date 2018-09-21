#include <iostream>
#include "call_stack_manager.h"

static const int kBitmapSize = 65536;
static const int kStride = 8;

namespace qsym {

  static bool isPowerOfTwo(uint16_t x) {
    return (x & (x - 1)) == 0;
  }

  CallStackManager::CallStackManager()
  : call_stack_()
  , is_interesting_(true)
  , bitmap_(new uint16_t[kBitmapSize])
  , last_index_(0)
  , pending_(false)
  , last_pc_(0)
  {}

  CallStackManager::~CallStackManager() {
    delete [] bitmap_;
  }

  void CallStackManager::visitCall(ADDRINT pc) {
    call_stack_.push_back(pc);
    computeHash();
  }

  void CallStackManager::visitRet(ADDRINT pc) {
    int num_elements_to_remove = 0;
    for (auto it = call_stack_.rbegin();
        it != call_stack_.rend();
        it++) {
      num_elements_to_remove += 1;
      if (call_stack_.back() == pc)
        break;
    }

    for (int i = 0; i < num_elements_to_remove; i++)
      call_stack_.pop_back();
    computeHash();
  }

  void CallStackManager::visitBasicBlock(ADDRINT pc) {
    last_pc_ = pc;
    pending_ = true;
  }

  void CallStackManager::updateBitmap() {
    // Lazy update the bitmap when symbolic operation is happened
    if (pending_) {
      pending_ = false;

      XXH32_state_t state;
      XXH32_reset(&state, 0);
      XXH32_update(&state, &last_pc_, sizeof(last_pc_));
      XXH32_update(&state, &call_stack_hash_, sizeof(call_stack_hash_));

      uint32_t h = XXH32_digest(&state);
      uint32_t index = h % kBitmapSize;

      // Use strided exponential backoff, which is interesting if the strided
      // bitmap meets exponential requirements. For example, {0, 1, 2, ..., 7}
      // maps to 0, {8, ..., 15} maps to 1, and so on. {0, 1, 2, ..., 7} is
      // interesting because it maps to 0, which is in the {0, 1, 2, 4, ...}.
      // But {24, ... 31} is not, because it maps to 3.
      is_interesting_ = isPowerOfTwo(bitmap_[index] / kStride);
      bitmap_[index]++;
    }
  }

  void CallStackManager::computeHash() {
    XXH32_state_t state;
    XXH32_reset(&state, 0);
    XXH32_update(&state, call_stack_.data(),
        sizeof(ADDRINT) * call_stack_.size());
    call_stack_hash_ = XXH32_digest(&state);
  }

} // namespace qsym
