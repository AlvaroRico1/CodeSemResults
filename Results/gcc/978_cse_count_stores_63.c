count_stores (rtx x, const_rtx set ATTRIBUTE_UNUSED, void *data)
{
  int *counts = (int *) data;
  if (REG_P (x) && REGNO (x) >= FIRST_PSEUDO_REGISTER)
    counts[REGNO (x)]++;
}


// Source: cse.c
// Lines 6997-7002
