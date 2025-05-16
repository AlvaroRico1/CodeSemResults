static void _state(struct Curl_easy *data,
                   ftpstate newstate
#ifdef DEBUGBUILD
                   , int lineno
#endif
  )
{
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;

#if defined(DEBUGBUILD)

#if defined(CURL_DISABLE_VERBOSE_STRINGS)
  (void) lineno;
#else
  if(ftpc->state != newstate)
    infof(data, "FTP %p (line %d) state change from %s to %s",
          (void *)ftpc, lineno, ftp_state_names[ftpc->state],
          ftp_state_names[newstate]);
#endif
#endif

  ftpc->state = newstate;
}


// Source: ftp.c
// Lines 755-778
