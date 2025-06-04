bitmap_single_bit_set_p (const_bitmap a)
{
  unsigned long count = 0;
  const bitmap_element *elt;
  unsigned ix;

  if (bitmap_empty_p (a))
    return false;

  elt = a->first;

  /* As there are no completely empty bitmap elements, a second one
     means we have more than one bit set.  */
  if (elt->next != NULL
      && (!a->tree_form || elt->prev != NULL))
    return false;

  for (ix = 0; ix != BITMAP_ELEMENT_WORDS; ix++)
    {
#if GCC_VERSION >= 3400
      /* Note that popcountl matches BITMAP_WORD in type, so the actual size
	 of BITMAP_WORD is not material.  */
      count += __builtin_popcountl (elt->bits[ix]);
#else
      count += bitmap_popcount (elt->bits[ix]);
#endif
      if (count > 1)
	return false;
    }

  return count == 1;
}


// Source: bitmap.c
// Lines 1103-1134
