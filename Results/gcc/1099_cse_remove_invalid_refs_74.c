remove_invalid_refs (unsigned int regno)
{
  unsigned int i;
  struct table_elt *p, *next;

  for (i = 0; i < HASH_SIZE; i++)
    for (p = table[i]; p; p = next)
      {
	next = p->next_same_hash;
	if (!REG_P (p->exp) && refers_to_regno_p (regno, p->exp))
	  remove_from_table (p, i);
      }
}


// Source: cse.c
// Lines 1986-1998
