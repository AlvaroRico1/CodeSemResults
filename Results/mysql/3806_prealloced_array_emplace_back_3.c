  bool emplace_back(Args &&... args) {
    const size_t expansion_factor = 2;
    if (m_size == m_capacity && reserve(m_capacity * expansion_factor))
      return true;
    Element_type *p = &m_array_ptr[m_size++];
    ::new (p) Element_type(std::forward<Args>(args)...);
    return false;
  }


// Source: prealloced_array.h
// Lines 321-328
