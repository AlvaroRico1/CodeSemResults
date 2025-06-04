push_to_sequence (rtx_insn *first)
{
  rtx_insn *last;

  start_sequence ();

  for (last = first; last && NEXT_INSN (last); last = NEXT_INSN (last))
    ;

  set_first_insn (first);
  set_last_insn (last);
}


// Source: emit-rtl.c
// Lines 5523-5534
