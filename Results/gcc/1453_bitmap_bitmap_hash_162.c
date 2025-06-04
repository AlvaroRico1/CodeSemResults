bitmap_hash (const_bitmap head)
{
  const bitmap_element *ptr;
  BITMAP_WORD hash = 0;
  int ix;

  gcc_checking_assert (!head->tree_form);

  for (ptr = head->first; ptr; ptr = ptr->next)
    {
      hash ^= ptr->indx;
      for (ix = 0; ix != BITMAP_ELEMENT_WORDS; ix++)
	hash ^= ptr->bits[ix];
    }
  return (hashval_t)hash;
}


// Source: bitmap.c
// Lines 2580-2595
