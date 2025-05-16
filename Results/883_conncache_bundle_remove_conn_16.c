static int bundle_remove_conn(struct connectbundle *bundle,
                              struct connectdata *conn)
{
  struct Curl_llist_element *curr;

  curr = bundle->conn_list.head;
  while(curr) {
    if(curr->ptr == conn) {
      Curl_llist_remove(&bundle->conn_list, curr, NULL);
      bundle->num_connections--;
      conn->bundle = NULL;
      return 1; /* we removed a handle */
    }
    curr = curr->next;
  }
  DEBUGASSERT(0);
  return 0;
}


// Source: conncache.c
// Lines 88-105
