bitmap_clear (bitmap head)
{
  if (head->first == NULL)
    return;
  if (head->tree_form)
    {
      bitmap_element *e, *t;
      for (e = head->first; e->prev; e = e->prev)
	/* Loop to find the element with the smallest index.  */ ;
      t = bitmap_tree_splay (head, head->first, e->indx);
      gcc_checking_assert (t == e);
      head->first = t;
    }


// Source: bitmap.c
// Lines 708-720
