static inline int inline_mysql_socket_getsockopt(
#ifdef HAVE_PSI_SOCKET_INTERFACE
    const char *src_file, uint src_line,
#endif
    MYSQL_SOCKET mysql_socket, int level, int optname, SOCKBUF_T *optval,
    socklen_t *optlen) {
  int result;

#ifdef HAVE_PSI_SOCKET_INTERFACE
  if (mysql_socket.m_psi != nullptr) {
    if (mysql_socket.m_psi->m_enabled) {
      /* Instrumentation start */
      PSI_socket_locker *locker;
      PSI_socket_locker_state state;
      locker = PSI_SOCKET_CALL(start_socket_wait)(&state, mysql_socket.m_psi,
                                                  PSI_SOCKET_OPT, (size_t)0,
                                                  src_file, src_line);

      /* Instrumented code */
      result = getsockopt(mysql_socket.fd, level, optname, optval, optlen);

      /* Instrumentation end */
      if (locker != nullptr) {
        PSI_SOCKET_CALL(end_socket_wait)(locker, (size_t)0);
      }

      return result;
    }


// Source: mysql_socket.h
// Lines 882-909
