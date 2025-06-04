static inline int my_mutex_destroy(my_mutex_t *mp
#ifdef SAFE_MUTEX
                                   ,
                                   const char *file, uint line
#endif
) {
#ifdef SAFE_MUTEX
  assert(mp != nullptr);
  assert(mp->m_u.m_safe_ptr != nullptr);
  int rc = safe_mutex_destroy(mp->m_u.m_safe_ptr, file, line);
  free(mp->m_u.m_safe_ptr);
  mp->m_u.m_safe_ptr = nullptr;
  return rc;
#else
  return native_mutex_destroy(&mp->m_u.m_native);
#endif
}


// Source: thr_mutex.h
// Lines 224-240
