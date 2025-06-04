static inline void inline_mysql_memory_register(
#ifdef HAVE_PSI_MEMORY_INTERFACE
    const char *category, PSI_memory_info *info, int count)
#else
    const char *category MY_ATTRIBUTE((unused)),
    void *info MY_ATTRIBUTE((unused)), int count MY_ATTRIBUTE((unused)))
#endif
{
#ifdef HAVE_PSI_MEMORY_INTERFACE
  PSI_MEMORY_CALL(register_memory)(category, info, count);
#endif
}

/** @} (end of group psi_api_memory) */


// Source: mysql_memory.h
// Lines 60-73
