bitmap_intersect_p (const_bitmap a, const_bitmap b)
{
  const bitmap_element *a_elt;
  const bitmap_element *b_elt;
  unsigned ix;

  gcc_checking_assert (!a->tree_form && !b->tree_form);

  for (a_elt = a->first, b_elt = b->first;
       a_elt && b_elt;)
    {
      if (a_elt->indx < b_elt->indx)
	a_elt = a_elt->next;
      else if (b_elt->indx < a_elt->indx)
	b_elt = b_elt->next;
      else
	{
	  for (ix = 0; ix < BITMAP_ELEMENT_WORDS; ix++)
	    if (a_elt->bits[ix] & b_elt->bits[ix])
	      return true;
	  a_elt = a_elt->next;
	  b_elt = b_elt->next;
	}
    }
  return false;
}


// Source: bitmap.c
// Lines 2253-2278
