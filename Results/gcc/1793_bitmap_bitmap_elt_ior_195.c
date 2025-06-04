bitmap_elt_ior (bitmap dst, bitmap_element *dst_elt, bitmap_element *dst_prev,
		const bitmap_element *a_elt, const bitmap_element *b_elt,
		bool changed)
{
  gcc_assert (a_elt || b_elt);

  if (a_elt && b_elt && a_elt->indx == b_elt->indx)
    {
      /* Matching elts, generate A | B.  */
      unsigned ix;

      if (!changed && dst_elt && dst_elt->indx == a_elt->indx)
	{
	  for (ix = 0; ix < BITMAP_ELEMENT_WORDS; ix++)
	    {
	      BITMAP_WORD r = a_elt->bits[ix] | b_elt->bits[ix];
	      if (r != dst_elt->bits[ix])
		{
		  dst_elt->bits[ix] = r;
		  changed = true;
		}
	    }
	}
      else
	{
	  changed = true;
	  if (!dst_elt)
	    dst_elt = bitmap_list_insert_element_after (dst, dst_prev,
							a_elt->indx);
	  else
	    dst_elt->indx = a_elt->indx;
	  for (ix = 0; ix < BITMAP_ELEMENT_WORDS; ix++)
	    {
	      BITMAP_WORD r = a_elt->bits[ix] | b_elt->bits[ix];
	      dst_elt->bits[ix] = r;
	    }
	}
    }
  else
    {
      /* Copy a single element.  */
      const bitmap_element *src;

      if (!b_elt || (a_elt && a_elt->indx < b_elt->indx))
	src = a_elt;
      else
	src = b_elt;

      gcc_checking_assert (src);
      changed = bitmap_elt_copy (dst, dst_elt, dst_prev, src, changed);
    }


// Source: bitmap.c
// Lines 1888-1938
