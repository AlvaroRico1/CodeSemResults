bitmap_and_into (bitmap a, const_bitmap b)
{
  bitmap_element *a_elt = a->first;
  const bitmap_element *b_elt = b->first;
  bitmap_element *next;
  bool changed = false;

  gcc_checking_assert (!a->tree_form && !b->tree_form);

  if (a == b)
    return false;

  while (a_elt && b_elt)
    {
      if (a_elt->indx < b_elt->indx)
	{
	  next = a_elt->next;
	  bitmap_list_unlink_element (a, a_elt);
	  a_elt = next;
	  changed = true;
	}
      else if (b_elt->indx < a_elt->indx)
	b_elt = b_elt->next;
      else
	{
	  /* Matching elts, generate A &= B.  */
	  unsigned ix;
	  BITMAP_WORD ior = 0;

	  for (ix = 0; ix < BITMAP_ELEMENT_WORDS; ix++)
	    {
	      BITMAP_WORD r = a_elt->bits[ix] & b_elt->bits[ix];
	      if (a_elt->bits[ix] != r)
		changed = true;
	      a_elt->bits[ix] = r;
	      ior |= r;
	    }
	  next = a_elt->next;
	  if (!ior)
	    bitmap_list_unlink_element (a, a_elt);
	  a_elt = next;
	  b_elt = b_elt->next;
	}
    }

  if (a_elt)
    {
      changed = true;
      bitmap_elt_clear_from (a, a_elt);
    }

  gcc_checking_assert (!a->current == !a->first
		       && (!a->current || a->indx == a->current->indx));

  return changed;
}


// Source: bitmap.c
// Lines 1307-1362
