  void *Alloc(size_t length) MY_ATTRIBUTE((malloc)) {
    length = ALIGN_SIZE(length);

    // Skip the straight path if simulating OOM; it should always fail.
    DBUG_EXECUTE_IF("simulate_out_of_memory", return AllocSlow(length););

    // Fast path, used in the majority of cases. It would be faster here
    // (saving one register due to CSE) to instead test
    //
    //   m_current_free_start + length <= m_current_free_end
    //
    // but it would invoke undefined behavior, and in particular be prone
    // to wraparound on 32-bit platforms.
    if (static_cast<size_t>(m_current_free_end - m_current_free_start) >=
        length) {
      void *ret = m_current_free_start;
      m_current_free_start += length;
      return ret;
    }


// Source: my_alloc.h
// Lines 135-153
