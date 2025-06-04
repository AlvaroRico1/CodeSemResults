bitmap_move (bitmap to, bitmap from)
{
  gcc_assert (to->obstack == from->obstack);

  bitmap_clear (to);

  size_t sz = 0;
  if (GATHER_STATISTICS)
    {
      for (bitmap_element *e = to->first; e; e = e->next)
	sz += sizeof (bitmap_element);
      register_overhead (to, sz);
    }


// Source: bitmap.c
// Lines 888-900
