emit_insn (rtx x)
{
  rtx_insn *last = get_last_insn ();
  rtx_insn *insn;

  if (x == NULL_RTX)
    return last;

  switch (GET_CODE (x))
    {
    case DEBUG_INSN:
    case INSN:
    case JUMP_INSN:
    case CALL_INSN:
    case CODE_LABEL:
    case BARRIER:
    case NOTE:
      insn = as_a <rtx_insn *> (x);
      while (insn)
	{
	  rtx_insn *next = NEXT_INSN (insn);
	  add_insn (insn);
	  last = insn;
	  insn = next;
	}


// Source: emit-rtl.c
// Lines 5057-5081
