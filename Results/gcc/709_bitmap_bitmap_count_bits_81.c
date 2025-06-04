bitmap_count_bits (const_bitmap a)
{
  unsigned long count = 0;
  const bitmap_element *elt;

  gcc_checking_assert (!a->tree_form);
  for (elt = a->first; elt; elt = elt->next)
    count += bitmap_count_bits_in_word (elt->bits);

  return count;
}


// Source: bitmap.c
// Lines 1051-1061
