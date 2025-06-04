static inline int inline_mysql_mutex_lock(
    mysql_mutex_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    uint src_line MY_ATTRIBUTE((unused))) {
  int result;

#ifdef HAVE_PSI_MUTEX_INTERFACE
  if (that->m_psi != nullptr) {
    if (that->m_psi->m_enabled) {
      /* Instrumentation start */
      PSI_mutex_locker *locker;
      PSI_mutex_locker_state state;
      locker = PSI_MUTEX_CALL(start_mutex_wait)(
          &state, that->m_psi, PSI_MUTEX_LOCK, src_file, src_line);

      /* Instrumented code */
      result = my_mutex_lock(&that->m_mutex
#ifdef SAFE_MUTEX
                             ,
                             src_file, src_line
#endif
      );

      /* Instrumentation end */
      if (locker != nullptr) {
        PSI_MUTEX_CALL(end_mutex_wait)(locker, result);
      }

      return result;
    }
  }
#endif

  /* Non instrumented code */
  result = my_mutex_lock(&that->m_mutex
#ifdef SAFE_MUTEX
                         ,
                         src_file, src_line
#endif
  );

  return result;
}


// Source: mysql_mutex.h
// Lines 238-279
