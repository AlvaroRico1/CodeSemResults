debug_bitmap_file (FILE *file, const_bitmap head)
{
  const bitmap_element *ptr;

  fprintf (file, "\nfirst = " HOST_PTR_PRINTF
	   " current = " HOST_PTR_PRINTF " indx = %u\n",
	   (void *) head->first, (void *) head->current, head->indx);

  if (head->tree_form)
    {
      auto_vec<bitmap_element *, 32> elts;
      bitmap_tree_to_vec (elts, head);
      for (unsigned i = 0; i < elts.length (); ++i)
	debug_bitmap_elt_file (file, elts[i]);
    }
  else
    for (ptr = head->first; ptr; ptr = ptr->next)
      debug_bitmap_elt_file (file, ptr);
}


// Source: bitmap.c
// Lines 2656-2674
