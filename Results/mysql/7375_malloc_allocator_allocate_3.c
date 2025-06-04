  pointer allocate(size_type n,
                   const_pointer hint MY_ATTRIBUTE((unused)) = nullptr) {
    if (n == 0) return nullptr;
    if (n > max_size()) throw std::bad_alloc();

    pointer p = static_cast<pointer>(
        my_malloc(m_key, n * sizeof(T), MYF(MY_WME | ME_FATALERROR)));
    if (p == nullptr) throw std::bad_alloc();
    return p;
  }


// Source: malloc_allocator.h
// Lines 92-101
