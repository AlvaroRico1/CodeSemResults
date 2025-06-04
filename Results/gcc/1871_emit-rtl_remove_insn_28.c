remove_insn (rtx_insn *insn)
{
  rtx_insn *next = NEXT_INSN (insn);
  rtx_insn *prev = PREV_INSN (insn);
  basic_block bb;

  if (prev)
    {
      SET_NEXT_INSN (prev) = next;
      if (NONJUMP_INSN_P (prev) && GET_CODE (PATTERN (prev)) == SEQUENCE)
	{
	  rtx_sequence *sequence = as_a <rtx_sequence *> (PATTERN (prev));
	  SET_NEXT_INSN (sequence->insn (sequence->len () - 1)) = next;
	}
    }
  else
    {
      struct sequence_stack *seq;

      for (seq = get_current_sequence (); seq; seq = seq->next)
	if (insn == seq->first)
	  {
	    seq->first = next;
	    break;
	  }

      gcc_assert (seq);
    }

  if (next)
    {
      SET_PREV_INSN (next) = prev;
      if (NONJUMP_INSN_P (next) && GET_CODE (PATTERN (next)) == SEQUENCE)
	{
	  rtx_sequence *sequence = as_a <rtx_sequence *> (PATTERN (next));
	  SET_PREV_INSN (sequence->insn (0)) = prev;
	}
    }
  else
    {
      struct sequence_stack *seq;

      for (seq = get_current_sequence (); seq; seq = seq->next)
	if (insn == seq->last)
	  {
	    seq->last = prev;
	    break;
	  }

      gcc_assert (seq);
    }


// Source: emit-rtl.c
// Lines 4298-4348
