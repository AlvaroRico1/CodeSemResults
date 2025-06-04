bitmap_alloc (bitmap_obstack *bit_obstack MEM_STAT_DECL)
{
  bitmap map;

  if (!bit_obstack)
    bit_obstack = &bitmap_default_obstack;
  map = bit_obstack->heads;
  if (map)
    bit_obstack->heads = (class bitmap_head *) map->first;
  else
    map = XOBNEW (&bit_obstack->obstack, bitmap_head);
  bitmap_initialize (map, bit_obstack PASS_MEM_STAT);

  if (GATHER_STATISTICS)
    register_overhead (map, sizeof (bitmap_head));

  return map;
}


// Source: bitmap.c
// Lines 774-791
