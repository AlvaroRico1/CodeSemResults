bitmap_xor (bitmap dst, const_bitmap a, const_bitmap b)
{
  bitmap_element *dst_elt = dst->first;
  const bitmap_element *a_elt = a->first;
  const bitmap_element *b_elt = b->first;
  bitmap_element *dst_prev = NULL;

  gcc_checking_assert (!dst->tree_form && !a->tree_form && !b->tree_form);
  gcc_assert (dst != a && dst != b);

  if (a == b)
    {
      bitmap_clear (dst);
      return;
    }

  while (a_elt || b_elt)
    {
      if (a_elt && b_elt && a_elt->indx == b_elt->indx)
	{
	  /* Matching elts, generate A ^ B.  */
	  unsigned ix;
	  BITMAP_WORD ior = 0;

	  if (!dst_elt)
	    dst_elt = bitmap_list_insert_element_after (dst, dst_prev,
							a_elt->indx);
	  else
	    dst_elt->indx = a_elt->indx;
	  for (ix = 0; ix < BITMAP_ELEMENT_WORDS; ix++)
	    {
	      BITMAP_WORD r = a_elt->bits[ix] ^ b_elt->bits[ix];

	      ior |= r;
	      dst_elt->bits[ix] = r;
	    }
	  a_elt = a_elt->next;
	  b_elt = b_elt->next;
	  if (ior)
	    {
	      dst_prev = dst_elt;
	      dst_elt = dst_elt->next;
	    }
	}
      else
	{
	  /* Copy a single element.  */
	  const bitmap_element *src;

	  if (!b_elt || (a_elt && a_elt->indx < b_elt->indx))
	    {
	      src = a_elt;
	      a_elt = a_elt->next;
	    }
	  else
	    {
	      src = b_elt;
	      b_elt = b_elt->next;
	    }

	  if (!dst_elt)
	    dst_elt = bitmap_list_insert_element_after (dst, dst_prev,
							src->indx);
	  else
	    dst_elt->indx = src->indx;
	  memcpy (dst_elt->bits, src->bits, sizeof (dst_elt->bits));
	  dst_prev = dst_elt;
	  dst_elt = dst_elt->next;
	}


// Source: bitmap.c
// Lines 2086-2154
