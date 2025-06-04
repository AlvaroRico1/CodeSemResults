emit_jump_insn (rtx x)
{
  rtx_insn *last = NULL;
  rtx_insn *insn;

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
// Lines 5151-5172
