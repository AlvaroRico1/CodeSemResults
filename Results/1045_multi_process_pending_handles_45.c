// Source: curl/lib/multi.c
// Lines 3497-3499
static void process_pending_handles(struct Curl_multi *multi)
{
  struct Curl_llist_element *e = multi->pending.head;
  if(e) {
    struct Curl_easy *data = e->ptr;

    DEBUGASSERT(data->mstate == MSTATE_PENDING);

    multistate(data, MSTATE_CONNECT);

    /* Remove this node from the list */
    Curl_llist_remove(&multi->pending, e, NULL);

    /* Make sure that the handle will be processed soonish. */
    Curl_expire(data, 0, EXPIRE_RUN_NOW);

    /* mark this as having been in the pending queue */
    data->state.previouslypending = TRUE;
  }
