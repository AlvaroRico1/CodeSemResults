// Source: curl/lib/conncache.c
// Lines 46-48
static void conn_llist_dtor(void *user, void *element)
{
  struct connectdata *conn = element;
  (void)user;
  conn->bundle = NULL;
}
