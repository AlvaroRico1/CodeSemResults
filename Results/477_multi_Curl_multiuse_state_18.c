void Curl_multiuse_state(struct Curl_easy *data,
                         int bundlestate) /* use BUNDLE_* defines */
{
  struct connectdata *conn;
  DEBUGASSERT(data);
  DEBUGASSERT(data->multi);
  conn = data->conn;
  DEBUGASSERT(conn);
  DEBUGASSERT(conn->bundle);

  conn->bundle->multiuse = bundlestate;
  process_pending_handles(data->multi);
}


// Source: multi.c
// Lines 3481-3493
