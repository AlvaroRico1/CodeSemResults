bitmap_xor_into (bitmap a, const_bitmap b)
{
  bitmap_element *a_elt = a->first;
  const bitmap_element *b_elt = b->first;
  bitmap_element *a_prev = NULL;

  gcc_checking_assert (!a->tree_form && !b->tree_form);

  if (a == b)
    {
      bitmap_clear (a);
      return;
    }

  while (b_elt)
    {
      if (!a_elt || b_elt->indx < a_elt->indx)
	{
	  /* Copy b_elt.  */
	  bitmap_element *dst = bitmap_list_insert_element_after (a, a_prev,
								  b_elt->indx);
	  memcpy (dst->bits, b_elt->bits, sizeof (dst->bits));
	  a_prev = dst;
	  b_elt = b_elt->next;
	}


// Source: bitmap.c
// Lines 2167-2191
