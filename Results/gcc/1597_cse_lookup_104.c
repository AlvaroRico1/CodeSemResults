lookup (rtx x, unsigned int hash, machine_mode mode)
{
  struct table_elt *p;

  for (p = table[hash]; p; p = p->next_same_hash)
    if (mode == p->mode && ((x == p->exp && REG_P (x))
			    || exp_equiv_p (x, p->exp, !REG_P (x), false)))
      return p;

  return 0;
}


// Source: cse.c
// Lines 1460-1470
