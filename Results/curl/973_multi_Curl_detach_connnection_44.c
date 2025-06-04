void Curl_detach_connnection(struct Curl_easy *data)
{
  struct connectdata *conn = data->conn;
  if(conn) {
    Curl_llist_remove(&conn->easyq, &data->conn_queue, NULL);
    Curl_ssl_detach_conn(data, conn);
  }
  data->conn = NULL;
}


// Source: multi.c
// Lines 877-885
