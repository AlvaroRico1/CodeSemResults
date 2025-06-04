bitmap_ior_into (bitmap a, const_bitmap b)
{
  bitmap_element *a_elt = a->first;
  const bitmap_element *b_elt = b->first;
  bitmap_element *a_prev = NULL;
  bitmap_element **a_prev_pnext = &a->first;
  bool changed = false;

  gcc_checking_assert (!a->tree_form && !b->tree_form);
  if (a == b)
    return false;

  while (b_elt)
    {
      /* If A lags behind B, just advance it.  */
      if (!a_elt || a_elt->indx == b_elt->indx)
	{
	  changed = bitmap_elt_ior (a, a_elt, a_prev, a_elt, b_elt, changed);
	  b_elt = b_elt->next;
	}
      else if (a_elt->indx > b_elt->indx)
	{
          changed = bitmap_elt_copy (a, NULL, a_prev, b_elt, changed);
	  b_elt = b_elt->next;
	}

      a_prev = *a_prev_pnext;
      a_prev_pnext = &a_prev->next;
      a_elt = *a_prev_pnext;
    }

  gcc_checking_assert (!a->current == !a->first);
  if (a->current)
    a->indx = a->current->indx;
  return changed;
}


// Source: bitmap.c
// Lines 1996-2031
