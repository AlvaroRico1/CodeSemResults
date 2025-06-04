void Curl_expire_clear(struct Curl_easy *data)
{
  struct Curl_multi *multi = data->multi;
  struct curltime *nowp = &data->state.expiretime;

  /* this is only interesting while there is still an associated multi struct
     remaining! */
  if(!multi)
    return;

  if(nowp->tv_sec || nowp->tv_usec) {
    /* Since this is an cleared time, we must remove the previous entry from
       the splay tree */
    struct Curl_llist *list = &data->state.timeoutlist;
    int rc;

    rc = Curl_splayremove(multi->timetree, &data->state.timenode,
                          &multi->timetree);
    if(rc)
      infof(data, "Internal error clearing splay node = %d", rc);

    /* flush the timeout list too */
    while(list->size > 0) {
      Curl_llist_remove(list, list->tail, NULL);
    }

#ifdef DEBUGBUILD
    infof(data, "Expire cleared (transfer %p)", data);
#endif
    nowp->tv_sec = 0;
    nowp->tv_usec = 0;
  }
}


// Source: multi.c
// Lines 3412-3444
