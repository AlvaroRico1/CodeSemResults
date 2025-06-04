std::pair<MEM_ROOT::Block *, size_t> MEM_ROOT::AllocBlock(
    size_t wanted_length, size_t minimum_length) {
  DBUG_TRACE;

  size_t length = wanted_length;
  if (m_max_capacity != 0) {
    size_t bytes_left;
    if (m_allocated_size > m_max_capacity) {
      bytes_left = 0;
    } else {
      bytes_left = m_max_capacity - m_allocated_size;
    }
    if (wanted_length > bytes_left) {
      if (m_error_for_capacity_exceeded) {
        my_error(EE_CAPACITY_EXCEEDED, MYF(0),
                 static_cast<ulonglong>(m_max_capacity));
        // NOTE: No early return; we will abort the query at the next safe
        // point. We also don't go down to minimum_length, as this will give a
        // new block on every subsequent Alloc() (of which there might be
        // many, since we don't know when the next safe point will be).
      } else if (minimum_length <= bytes_left) {
        // Make one final chunk with all that we have left.
        length = bytes_left;
      } else {
        // We don't have enough memory left to satisfy minimum_length.
        return {nullptr, 0};
      }
    }
  }

  Block *new_block = static_cast<Block *>(
      my_malloc(m_psi_key, length + ALIGN_SIZE(sizeof(Block)),
                MYF(MY_WME | ME_FATALERROR)));
  if (new_block == nullptr) {
    if (m_error_handler) (m_error_handler)();
    return {nullptr, 0};
  }

  m_allocated_size += length;

  // Make the default block size 50% larger next time.
  // This ensures O(1) total mallocs (assuming Clear() is not called).
  m_block_size += m_block_size / 2;
  return {new_block, length};
}


// Source: my_alloc.cc
// Lines 56-100
