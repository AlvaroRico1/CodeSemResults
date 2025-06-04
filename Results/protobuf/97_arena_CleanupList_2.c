void SerialArena::CleanupList() {
  Block* b = head_;
  b->start = reinterpret_cast<CleanupNode*>(limit_);
  do {
    auto* limit = reinterpret_cast<CleanupNode*>(
        b->Pointer(b->size & static_cast<size_t>(-8)));
    auto it = b->start;
    auto num = limit - it;
    if (num > 0) {
      for (; it < limit; it++) {
        it->cleanup(it->elem);
      }
    }
    b = b->next;
  } while (b);
}


// Source: arena.cc
// Lines 191-206
