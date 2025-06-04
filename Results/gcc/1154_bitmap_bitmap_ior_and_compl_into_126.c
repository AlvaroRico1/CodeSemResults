bitmap_ior_and_compl_into (bitmap a, const_bitmap b, const_bitmap c)
{
  bitmap_element *a_elt = a->first;
  const bitmap_element *b_elt = b->first;
  const bitmap_element *c_elt = c->first;
  bitmap_element and_elt;
  bitmap_element *a_prev = NULL;
  bitmap_element **a_prev_pnext = &a->first;
  bool changed = false;
  unsigned ix;

  gcc_checking_assert (!a->tree_form && !b->tree_form && !c->tree_form);

  if (a == b)
    return false;
  if (bitmap_empty_p (c))
    return bitmap_ior_into (a, b);
  else if (bitmap_empty_p (a))
    return bitmap_and_compl (a, b, c);

  and_elt.indx = -1;
  while (b_elt)
    {
      /* Advance C.  */
      while (c_elt && c_elt->indx < b_elt->indx)
	c_elt = c_elt->next;

      const bitmap_element *and_elt_ptr;
      if (c_elt && c_elt->indx == b_elt->indx)
	{
	  BITMAP_WORD overall = 0;
	  and_elt_ptr = &and_elt;
	  and_elt.indx = b_elt->indx;
	  for (ix = 0; ix < BITMAP_ELEMENT_WORDS; ix++)
	    {
	      and_elt.bits[ix] = b_elt->bits[ix] & ~c_elt->bits[ix];
	      overall |= and_elt.bits[ix];
	    }
	  if (!overall)
	    {
	      b_elt = b_elt->next;
	      continue;
	    }
	}
      else
	and_elt_ptr = b_elt;

      b_elt = b_elt->next;

      /* Now find a place to insert AND_ELT.  */
      do
	{
	  ix = a_elt ? a_elt->indx : and_elt_ptr->indx;
          if (ix == and_elt_ptr->indx)
	    changed = bitmap_elt_ior (a, a_elt, a_prev, a_elt,
				      and_elt_ptr, changed);
          else if (ix > and_elt_ptr->indx)
	    changed = bitmap_elt_copy (a, NULL, a_prev, and_elt_ptr, changed);

          a_prev = *a_prev_pnext;
          a_prev_pnext = &a_prev->next;
          a_elt = *a_prev_pnext;

          /* If A lagged behind B/C, we advanced it so loop once more.  */
	}
      while (ix < and_elt_ptr->indx);
    }


// Source: bitmap.c
// Lines 2422-2488
