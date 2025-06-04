static inline int inline_mysql_mutex_unlock(
    mysql_mutex_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    uint src_line MY_ATTRIBUTE((unused))) {
  int result;

#ifdef HAVE_PSI_MUTEX_INTERFACE
  if (that->m_psi != nullptr) {
    PSI_MUTEX_CALL(unlock_mutex)(that->m_psi);
  }
#endif

  result = my_mutex_unlock(&that->m_mutex
#ifdef SAFE_MUTEX
                           ,
                           src_file, src_line
#endif
  );

  return result;
}


// Source: mysql_mutex.h
// Lines 324-343
