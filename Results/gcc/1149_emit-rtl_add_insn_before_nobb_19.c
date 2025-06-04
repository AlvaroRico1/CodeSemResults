add_insn_before_nobb (rtx_insn *insn, rtx_insn *before)
{
  rtx_insn *prev = PREV_INSN (before);

  gcc_assert (!optimize || !before->deleted ());

  link_insn_into_chain (insn, prev, before);

  if (prev == NULL)
    {
      struct sequence_stack *seq;

      for (seq = get_current_sequence (); seq; seq = seq->next)
	if (before == seq->first)
	  {
	    seq->first = insn;
	    break;
	  }

      gcc_assert (seq);
    }


// Source: emit-rtl.c
// Lines 4188-4208
