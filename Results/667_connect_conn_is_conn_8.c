static int conn_is_conn(struct Curl_easy *data,
                        struct connectdata *conn, void *param)
{
  struct connfind *f = (struct connfind *)param;
  (void)data;
  if(conn->connection_id == f->id_tofind) {
    f->found = conn;
    return 1;
  }
  return 0;
}


// Source: connect.c
// Lines 1432-1442
