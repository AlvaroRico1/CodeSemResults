lookup_for_remove (rtx x, unsigned int hash, machine_mode mode)
{
  struct table_elt *p;

  if (REG_P (x))
    {
      unsigned int regno = REGNO (x);

      /* Don't check the machine mode when comparing registers;
	 invalidating (REG:SI 0) also invalidates (REG:DF 0).  */
      for (p = table[hash]; p; p = p->next_same_hash)
	if (REG_P (p->exp)
	    && REGNO (p->exp) == regno)
	  return p;
    }
  else
    {
      for (p = table[hash]; p; p = p->next_same_hash)
	if (mode == p->mode
	    && (x == p->exp || exp_equiv_p (x, p->exp, 0, false)))
	  return p;
    }

  return 0;
}


// Source: cse.c
// Lines 1476-1500
