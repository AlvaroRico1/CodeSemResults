get_last_insn_anywhere (void)
{
  struct sequence_stack *seq;
  for (seq = get_current_sequence (); seq; seq = seq->next)
    if (seq->last != 0)
      return seq->last;
  return 0;
}


// Source: emit-rtl.c
// Lines 3329-3336
