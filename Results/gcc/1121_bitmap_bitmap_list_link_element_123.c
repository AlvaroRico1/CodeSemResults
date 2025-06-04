bitmap_list_link_element (bitmap head, bitmap_element *element)
{
  unsigned int indx = element->indx;
  bitmap_element *ptr;

  gcc_checking_assert (!head->tree_form);

  /* If this is the first and only element, set it in.  */
  if (head->first == 0)
    {
      element->next = element->prev = 0;
      head->first = element;
    }

  /* If this index is less than that of the current element, it goes someplace
     before the current element.  */
  else if (indx < head->indx)
    {
      for (ptr = head->current;
	   ptr->prev != 0 && ptr->prev->indx > indx;
	   ptr = ptr->prev)
	;

      if (ptr->prev)
	ptr->prev->next = element;
      else
	head->first = element;

      element->prev = ptr->prev;
      element->next = ptr;
      ptr->prev = element;
    }

  /* Otherwise, it must go someplace after the current element.  */
  else
    {
      for (ptr = head->current;
	   ptr->next != 0 && ptr->next->indx < indx;
	   ptr = ptr->next)
	;

      if (ptr->next)
	ptr->next->prev = element;

      element->next = ptr->next;
      element->prev = ptr;
      ptr->next = element;
    }

  /* Set up so this is the first element searched.  */
  head->current = element;
  head->indx = indx;
}

/* Unlink the bitmap element from the current bitmap linked list,
   and return it to the freelist.  */

static inline void
bitmap_list_unlink_element (bitmap head, bitmap_element *element,
			    bool to_freelist = true)
{
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
// Lines 212-300
