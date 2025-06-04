invalidate_memory (void)
{
  int i;
  struct table_elt *p, *next;

  for (i = 0; i < HASH_SIZE; i++)
    for (p = table[i]; p; p = next)
      {
	next = p->next_same_hash;
	if (p->in_memory)
	  remove_from_table (p, i);
      }
}


// Source: cse.c
// Lines 6117-6129
