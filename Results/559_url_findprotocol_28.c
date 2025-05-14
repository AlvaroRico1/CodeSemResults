// Source: curl/lib/url.c
// Lines 1814-1817
static CURLcode findprotocol(struct Curl_easy *data,
                             struct connectdata *conn,
                             const char *protostr)
{
  const struct Curl_handler *p = Curl_builtin_scheme(protostr);

  if(p && /* Protocol found in table. Check if allowed */
     (data->set.allowed_protocols & p->protocol)) {

    /* it is allowed for "normal" request, now do an extra check if this is
       the result of a redirect */
    if(data->state.this_is_a_follow &&
       !(data->set.redir_protocols & p->protocol))
      /* nope, get out */
      ;
    else {
      /* Perform setup complement if some. */
      conn->handler = conn->given = p;

      /* 'port' and 'remote_port' are set in setup_connection_internals() */
      return CURLE_OK;
    }
  }

  /* The protocol was not found in the table, but we don't have to assign it
     to anything since it is already assigned to a dummy-struct in the
     create_conn() function when the connectdata struct is allocated. */
  failf(data, "Protocol \"%s\" not supported or disabled in " LIBCURL_NAME,
        protostr);

  return CURLE_UNSUPPORTED_PROTOCOL;
}
