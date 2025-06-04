reorder_insns_nobb (rtx_insn *from, rtx_insn *to, rtx_insn *after)
{
  if (flag_checking)
    {
      for (rtx_insn *x = from; x != to; x = NEXT_INSN (x))
	gcc_assert (after != x);
      gcc_assert (after != to);
    }


// Source: emit-rtl.c
// Lines 4413-4420
