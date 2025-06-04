bitmap_tree_unlink_element (bitmap head, bitmap_element *e)
{
  bitmap_element *t = bitmap_tree_splay (head, head->first, e->indx);

  gcc_checking_assert (t == e);

  if (e->prev == NULL)
    t = e->next;
  else
    {
      t = bitmap_tree_splay (head, e->prev, e->indx);
      t->next = e->next;
    }
  head->first = t;
  head->current = t;
  head->indx = (t != NULL) ? t->indx : 0;

  bitmap_elem_to_freelist (head, e);
}


// Source: bitmap.c
// Lines 540-558
