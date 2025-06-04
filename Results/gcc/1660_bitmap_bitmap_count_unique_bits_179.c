bitmap_count_unique_bits (const_bitmap a, const_bitmap b)
{
  unsigned long count = 0;
  const bitmap_element *elt_a, *elt_b;

  for (elt_a = a->first, elt_b = b->first; elt_a && elt_b; )
    {
      /* If we're at different indices, then count all the bits
	 in the lower element.  If we're at the same index, then
	 count the bits in the IOR of the two elements.  */
      if (elt_a->indx < elt_b->indx)
	{
	  count += bitmap_count_bits_in_word (elt_a->bits);
	  elt_a = elt_a->next;
	}
      else if (elt_b->indx < elt_a->indx)
	{
	  count += bitmap_count_bits_in_word (elt_b->bits);
	  elt_b = elt_b->next;
	}
      else
	{
	  BITMAP_WORD bits[BITMAP_ELEMENT_WORDS];
	  for (unsigned ix = 0; ix != BITMAP_ELEMENT_WORDS; ix++)
	    bits[ix] = elt_a->bits[ix] | elt_b->bits[ix];
	  count += bitmap_count_bits_in_word (bits);
	  elt_a = elt_a->next;
	  elt_b = elt_b->next;
	}
    }
  return count;
}


// Source: bitmap.c
// Lines 1066-1097
