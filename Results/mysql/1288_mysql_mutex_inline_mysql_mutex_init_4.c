static inline int inline_mysql_mutex_init(
    PSI_mutex_key key MY_ATTRIBUTE((unused)), mysql_mutex_t *that,
    const native_mutexattr_t *attr, const char *src_file MY_ATTRIBUTE((unused)),
    uint src_line MY_ATTRIBUTE((unused))) {
#ifdef HAVE_PSI_MUTEX_INTERFACE
  that->m_psi = PSI_MUTEX_CALL(init_mutex)(key, &that->m_mutex);
#else
  that->m_psi = nullptr;
#endif
  return my_mutex_init(&that->m_mutex, attr
#ifdef SAFE_MUTEX
                       ,
                       src_file, src_line
#endif
  );
}

static inline int inline_mysql_mutex_destroy(
    mysql_mutex_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    uint src_line MY_ATTRIBUTE((unused))) {
#ifdef HAVE_PSI_MUTEX_INTERFACE
  if (that->m_psi != nullptr) {
    PSI_MUTEX_CALL(destroy_mutex)(that->m_psi);
    that->m_psi = nullptr;
  }
#endif
  return my_mutex_destroy(&that->m_mutex
#ifdef SAFE_MUTEX
                          ,
                          src_file, src_line
#endif
  );
}

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

static inline int inline_mysql_mutex_trylock(
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
          &state, that->m_psi, PSI_MUTEX_TRYLOCK, src_file, src_line);

      /* Instrumented code */
      result = my_mutex_trylock(&that->m_mutex
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
  result = my_mutex_trylock(&that->m_mutex
#ifdef SAFE_MUTEX
                            ,
                            src_file, src_line
#endif
  );

  return result;
}

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

#endif /* DISABLE_MYSQL_THREAD_H */

/** @} (end of group psi_api_mutex) */


// Source: mysql_mutex.h
// Lines 204-347
