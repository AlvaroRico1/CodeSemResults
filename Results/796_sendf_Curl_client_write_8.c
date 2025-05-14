// Source: curl/lib/sendf.c
// Lines 617-622
CURLcode Curl_client_write(struct Curl_easy *data,
                           int type,
                           char *ptr,
                           size_t len)
{
  struct connectdata *conn = data->conn;

  DEBUGASSERT(len);
  DEBUGASSERT(type <= 3);

  /* FTP data may need conversion. */
  if((type & CLIENTWRITE_BODY) &&
    (conn->handler->protocol & PROTO_FAMILY_FTP) &&
    conn->proto.ftpc.transfertype == 'A') {
    /* convert from the network encoding */
    CURLcode result = Curl_convert_from_network(data, ptr, len);
    /* Curl_convert_from_network calls failf if unsuccessful */
    if(result)
      return result;

#ifdef CURL_DO_LINEEND_CONV
    /* convert end-of-line markers */
    len = convert_lineends(data, ptr, len);
#endif /* CURL_DO_LINEEND_CONV */
    }

  return chop_write(data, type, ptr, len);
}
