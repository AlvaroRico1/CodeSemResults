remove_invalid_subreg_refs (unsigned int regno, poly_uint64 offset,
			    machine_mode mode)
{
  unsigned int i;
  struct table_elt *p, *next;

  for (i = 0; i < HASH_SIZE; i++)
    for (p = table[i]; p; p = next)
      {
	rtx exp = p->exp;
	next = p->next_same_hash;

	if (!REG_P (exp)
	    && (GET_CODE (exp) != SUBREG
		|| !REG_P (SUBREG_REG (exp))
		|| REGNO (SUBREG_REG (exp)) != regno
		|| ranges_maybe_overlap_p (SUBREG_BYTE (exp),
					   GET_MODE_SIZE (GET_MODE (exp)),
					   offset, GET_MODE_SIZE (mode)))
	    && refers_to_regno_p (regno, p->exp))
	  remove_from_table (p, i);
      }
}


// Source: cse.c
// Lines 2003-2025
