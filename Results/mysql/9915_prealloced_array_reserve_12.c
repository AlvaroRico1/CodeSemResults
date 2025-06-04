  bool reserve(size_t n) {
    if (n <= m_capacity) return false;

    void *mem = my_malloc(m_psi_key, n * element_size(), MYF(MY_WME));
    if (!mem) return true;
    Element_type *new_array = static_cast<Element_type *>(mem);

    // Move all the existing elements into the new array.
    for (size_t ix = 0; ix < m_size; ++ix) {
      Element_type *new_p = &new_array[ix];
      Element_type &old_p = m_array_ptr[ix];
      ::new (new_p) Element_type(std::move(old_p));  // Move into new location.
      if (!Has_trivial_destructor)
        old_p.~Element_type();  // Destroy the old element.
    }

    if (m_array_ptr != cast_rawbuff()) my_free(m_array_ptr);

    // Forget the old array;
    m_array_ptr = new_array;
    m_capacity = n;
    return false;
  }


// Source: prealloced_array.h
// Lines 275-297
