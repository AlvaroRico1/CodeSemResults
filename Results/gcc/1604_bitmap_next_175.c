  bitmap_element *next = element->next;
  bitmap_element *prev = element->prev;

  gcc_checking_assert (!head->tree_form);

  if (prev)
    prev->next = next;

  if (next)
    next->prev = prev;

  if (head->first == element)
    head->first = next;

  /* Since the first thing we try is to insert before current,
     make current the next entry in preference to the previous.  */
  if (head->current == element)
    {
      head->current = next != 0 ? next : prev;
      if (head->current)
	head->indx = head->current->indx;
      else
	head->indx = 0;
    }

  if (to_freelist)
    bitmap_elem_to_freelist (head, element);
}


// Source: bitmap.c
// Lines 273-300
