// Source: curl/lib/pingpong.c
// Lines 166-172
CURLcode Curl_pp_vsendf(struct Curl_easy *data,
                        struct pingpong *pp,
                        const char *fmt,
                        va_list args)
{
  ssize_t bytes_written = 0;
  size_t write_len;
  char *s;
  CURLcode result;
  struct connectdata *conn = data->conn;

#ifdef HAVE_GSSAPI
  enum protection_level data_sec;
#endif

  DEBUGASSERT(pp->sendleft == 0);
  DEBUGASSERT(pp->sendsize == 0);
  DEBUGASSERT(pp->sendthis == NULL);

  if(!conn)
    /* can't send without a connection! */
    return CURLE_SEND_ERROR;

  Curl_dyn_reset(&pp->sendbuf);
  result = Curl_dyn_vaddf(&pp->sendbuf, fmt, args);
  if(result)
    return result;

  /* append CRLF */
  result = Curl_dyn_addn(&pp->sendbuf, "\r\n", 2);
  if(result)
    return result;

  write_len = Curl_dyn_len(&pp->sendbuf);
  s = Curl_dyn_ptr(&pp->sendbuf);
  Curl_pp_init(data, pp);

  result = Curl_convert_to_network(data, s, write_len);
  /* Curl_convert_to_network calls failf if unsuccessful */
  if(result)
    return result;

#ifdef HAVE_GSSAPI
  conn->data_prot = PROT_CMD;
#endif
  result = Curl_write(data, conn->sock[FIRSTSOCKET], s, write_len,
                      &bytes_written);
  if(result)
    return result;
#ifdef HAVE_GSSAPI
  data_sec = conn->data_prot;
  DEBUGASSERT(data_sec > PROT_NONE && data_sec < PROT_LAST);
  conn->data_prot = data_sec;
#endif

  Curl_debug(data, CURLINFO_HEADER_OUT, s, (size_t)bytes_written);

  if(bytes_written != (ssize_t)write_len) {
    /* the whole chunk was not sent, keep it around and adjust sizes */
    pp->sendthis = s;
    pp->sendsize = write_len;
    pp->sendleft = write_len - bytes_written;
  }
  else {
    pp->sendthis = NULL;
    pp->sendleft = pp->sendsize = 0;
    pp->response = Curl_now();
  }

  return CURLE_OK;
}
