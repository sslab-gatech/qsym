#include "expr_cache.h"

namespace qsym {

  ExprCache::ExprCache() : limit_(kCacheSize) {}

  void ExprCache::insert(WeakExprRef e) {
    if (set_.insert(e).second)
      queue_.push(e);
    shrink();
  }

  ExprRef ExprCache::find(ExprRef e) {
    auto it = set_.find(e);
    if (it != set_.end())
      return it->lock();
    else
      return NULL;
  }

  void ExprCache::shrink() {
    if (queue_.size() > limit_) {
      WeakExprRef item = queue_.front();

      // if item is expired, we cannot find entry using item
      // so let cleanup() handle
      if (!item.expired()) {
        auto it = set_.find(queue_.front());
        if (it != set_.end())
          set_.erase(it);
      }
      queue_.pop();
    }

    if (set_.size() > limit_ * 16)
      cleanup();
  }

  void ExprCache::cleanup() {
    for (auto it = set_.begin(); it != set_.end(); ) {
      auto current = it++;
      if (current->expired())
        set_.erase(current);
    }
  }
} // namespace qsym
