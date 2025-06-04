bitmap_equal_p (const_bitmap a, const_bitmap b)
{
  const bitmap_element *a_elt;
  const bitmap_element *b_elt;
  unsigned ix;

  gcc_checking_assert (!a->tree_form && !b->tree_form);

  for (a_elt = a->first, b_elt = b->first;
       a_elt && b_elt;
       a_elt = a_elt->next, b_elt = b_elt->next)
    {
      if (a_elt->indx != b_elt->indx)
	return false;
      for (ix = 0; ix < BITMAP_ELEMENT_WORDS; ix++)
	if (a_elt->bits[ix] != b_elt->bits[ix])
	  return false;
    }
  return !a_elt && !b_elt;
}


// Source: bitmap.c
// Lines 2229-2248
