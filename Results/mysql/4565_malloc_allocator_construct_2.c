  void construct(U *p, Args &&... args) {
    assert(p != nullptr);
    try {
      ::new ((void *)p) U(std::forward<Args>(args)...);
    } catch (...) {
      assert(false);  // Constructor should not throw an exception.
    }
  }

  void destroy(pointer p) {
    assert(p != nullptr);
    try {
      p->~T();
    } catch (...) {
      assert(false);  // Destructor should not throw an exception
    }
  }

  size_type max_size() const {
    return std::numeric_limits<size_t>::max() / sizeof(T);
  }

  template <class U>
  struct rebind {
    typedef Malloc_allocator<U> other;
  };

  PSI_memory_key psi_key() const { return m_key; }
};


// Source: malloc_allocator.h
// Lines 106-134
