Curl_llist_remove(struct Curl_llist *list, struct Curl_llist_element *e,
                  void *user)
{
  void *ptr;
  if(!e || list->size == 0)
    return;

  if(e == list->head) {
    list->head = e->next;

    if(!list->head)
      list->tail = NULL;
    else
      e->next->prev = NULL;
  }
  else {
    if(!e->prev)
      list->head = e->next;
    else
      e->prev->next = e->next;

    if(!e->next)
      list->tail = e->prev;
    else
      e->next->prev = e->prev;
  }

  ptr = e->ptr;

  e->ptr  = NULL;
  e->prev = NULL;
  e->next = NULL;

  --list->size;

  /* call the dtor() last for when it actually frees the 'e' memory itself */
  if(list->dtor)
    list->dtor(user, ptr);
}


// Source: llist.c
// Lines 93-131
