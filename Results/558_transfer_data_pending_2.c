// Source: curl/lib/transfer.c
// Lines 491-493
static int data_pending(const struct Curl_easy *data)
{
  struct connectdata *conn = data->conn;

#ifdef ENABLE_QUIC
  if(conn->transport == TRNSPRT_QUIC)
    return Curl_quic_data_pending(data);
#endif

  if(conn->handler->protocol&PROTO_FAMILY_FTP)
    return Curl_ssl_data_pending(conn, SECONDARYSOCKET);

  /* in the case of libssh2, we can never be really sure that we have emptied
     its internal buffers so we MUST always try until we get EAGAIN back */
  return conn->handler->protocol&(CURLPROTO_SCP|CURLPROTO_SFTP) ||
#if defined(USE_NGHTTP2)
    /* For HTTP/2, we may read up everything including response body
       with header fields in Curl_http_readwrite_headers. If no
       content-length is provided, curl waits for the connection
       close, which we emulate it using conn->proto.httpc.closed =
       TRUE. The thing is if we read everything, then http2_recv won't
       be called and we cannot signal the HTTP/2 stream has closed. As
       a workaround, we return nonzero here to call http2_recv. */
    ((conn->handler->protocol&PROTO_FAMILY_HTTP) && conn->httpversion >= 20) ||
#endif
    Curl_ssl_data_pending(conn, FIRSTSOCKET);
}
