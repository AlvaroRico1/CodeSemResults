static int call_extract_if_dead(struct Curl_easy *data,
                                struct connectdata *conn, void *param)
{
  struct prunedead *p = (struct prunedead *)param;
  if(extract_if_dead(conn, data)) {
    /* stop the iteration here, pass back the connection that was extracted */
    p->extracted = conn;
    return 1;
  }
  return 0; /* continue iteration */
}


// Source: url.c
// Lines 1042-1052
