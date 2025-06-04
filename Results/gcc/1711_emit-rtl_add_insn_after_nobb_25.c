add_insn_after_nobb (rtx_insn *insn, rtx_insn *after)
{
  rtx_insn *next = NEXT_INSN (after);

  gcc_assert (!optimize || !after->deleted ());

  link_insn_into_chain (insn, after, next);

  if (next == NULL)
    {
      struct sequence_stack *seq;

      for (seq = get_current_sequence (); seq; seq = seq->next)
	if (after == seq->last)
	  {
	    seq->last = insn;
	    break;
	  }
    }


// Source: emit-rtl.c
// Lines 4164-4182
