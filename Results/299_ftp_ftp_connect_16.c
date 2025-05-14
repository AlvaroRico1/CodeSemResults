// Source: curl/lib/ftp.c
// Lines 3145-3149
static CURLcode ftp_connect(struct Curl_easy *data,
                            bool *done) /* see description above */
{
  CURLcode result;
  struct connectdata *conn = data->conn;
  struct ftp_conn *ftpc = &conn->proto.ftpc;
  struct pingpong *pp = &ftpc->pp;

  *done = FALSE; /* default to not done yet */

  /* We always support persistent connections on ftp */
  connkeep(conn, "FTP default");

  PINGPONG_SETUP(pp, ftp_statemachine, ftp_endofresp);

  if(conn->handler->flags & PROTOPT_SSL) {
    /* BLOCKING */
    result = Curl_ssl_connect(data, conn, FIRSTSOCKET);
    if(result)
      return result;
    conn->bits.ftp_use_control_ssl = TRUE;
  }

  Curl_pp_setup(pp); /* once per transfer */
  Curl_pp_init(data, pp); /* init the generic pingpong data */

  /* When we connect, we start in the state where we await the 220
     response */
  state(data, FTP_WAIT220);

  result = ftp_multi_statemach(data, done);

  return result;
}
