CURLcode Curl_input_ntlm(struct Curl_easy *data,
                         bool proxy,         /* if proxy or not */
                         const char *header) /* rest of the www-authenticate:
                                                header */
{
  /* point to the correct struct with this */
  struct ntlmdata *ntlm;
  curlntlm *state;
  CURLcode result = CURLE_OK;
  struct connectdata *conn = data->conn;

  ntlm = proxy ? &conn->proxyntlm : &conn->ntlm;
  state = proxy ? &conn->proxy_ntlm_state : &conn->http_ntlm_state;

  if(checkprefix("NTLM", header)) {
    header += strlen("NTLM");

    while(*header && ISSPACE(*header))
      header++;

    if(*header) {
      unsigned char *hdr;
      size_t hdrlen;

      result = Curl_base64_decode(header, &hdr, &hdrlen);
      if(!result) {
        struct bufref hdrbuf;

        Curl_bufref_init(&hdrbuf);
        Curl_bufref_set(&hdrbuf, hdr, hdrlen, curl_free);
        result = Curl_auth_decode_ntlm_type2_message(data, &hdrbuf, ntlm);
        Curl_bufref_free(&hdrbuf);
      }
      if(result)
        return result;

      *state = NTLMSTATE_TYPE2; /* We got a type-2 message */
    }
    else {
      if(*state == NTLMSTATE_LAST) {
        infof(data, "NTLM auth restarted");
        Curl_http_auth_cleanup_ntlm(conn);
      }
      else if(*state == NTLMSTATE_TYPE3) {
        infof(data, "NTLM handshake rejected");
        Curl_http_auth_cleanup_ntlm(conn);
        *state = NTLMSTATE_NONE;
        return CURLE_REMOTE_ACCESS_DENIED;
      }
      else if(*state >= NTLMSTATE_TYPE1) {
        infof(data, "NTLM handshake failure (internal error)");
        return CURLE_REMOTE_ACCESS_DENIED;
      }

      *state = NTLMSTATE_TYPE1; /* We should send away a type-1 */
    }
  }

  return result;
}


// Source: http_ntlm.c
// Lines 63-122
