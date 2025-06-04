bitmap_elem_to_freelist (bitmap head, bitmap_element *elt)
{
  bitmap_obstack *bit_obstack = head->obstack;

  if (GATHER_STATISTICS)
    release_overhead (head, sizeof (bitmap_element), false);

  elt->next = NULL;
  elt->indx = -1;
  if (bit_obstack)
    {
      elt->prev = bit_obstack->elements;
      bit_obstack->elements = elt;
    }
  else
    {
      elt->prev = bitmap_ggc_free;
      bitmap_ggc_free = elt;
    }
}


// Source: bitmap.c
// Lines 78-97
