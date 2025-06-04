lookup_as_function (rtx x, enum rtx_code code)
{
  struct table_elt *p
    = lookup (x, SAFE_HASH (x, VOIDmode), GET_MODE (x));

  if (p == 0)
    return 0;

  for (p = p->first_same_value; p; p = p->next_same_value)
    if (GET_CODE (p->exp) == code
	/* Make sure this is a valid entry in the table.  */
	&& exp_equiv_p (p->exp, p->exp, 1, false))
      return p->exp;

  return 0;
}


// Source: cse.c
// Lines 1506-1521
