// Source: curl/lib/socks.c
// Lines 110-117
static void socksstate(struct Curl_easy *data,
                       enum connect_t state
#ifdef DEBUG_AND_VERBOSE
                       , int lineno
#endif
)
{
  struct connectdata *conn = data->conn;
  enum connect_t oldstate = conn->cnnct.state;
#ifdef DEBUG_AND_VERBOSE
  /* synced with the state list in urldata.h */
  static const char * const statename[] = {
    "INIT",
    "SOCKS_INIT",
    "SOCKS_SEND",
    "SOCKS_READ_INIT",
    "SOCKS_READ",
    "GSSAPI_INIT",
    "AUTH_INIT",
    "AUTH_SEND",
    "AUTH_READ",
    "REQ_INIT",
    "RESOLVING",
    "RESOLVED",
    "RESOLVE_REMOTE",
    "REQ_SEND",
    "REQ_SENDING",
    "REQ_READ",
    "REQ_READ_MORE",
    "DONE"
  };
#endif

  if(oldstate == state)
    /* don't bother when the new state is the same as the old state */
    return;

  conn->cnnct.state = state;

#ifdef DEBUG_AND_VERBOSE
  infof(data,
        "SXSTATE: %s => %s conn %p; line %d",
        statename[oldstate], statename[conn->cnnct.state], conn,
        lineno);
#endif
}
