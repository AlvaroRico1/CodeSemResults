emit_pattern_before_noloc (rtx x, rtx_insn *before, rtx_insn *last,
			   basic_block bb,
                           rtx_insn *(*make_raw) (rtx))
{
  rtx_insn *insn;

  gcc_assert (before);

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
	  add_insn_before (insn, before, bb);
	  last = insn;
	  insn = next;
	}
      break;

#ifdef ENABLE_RTL_CHECKING
    case SEQUENCE:
      gcc_unreachable ();
      break;
#endif

    default:
      last = (*make_raw) (x);
      add_insn_before (last, before, bb);
      break;
    }

  return last;
}


// Source: emit-rtl.c
// Lines 4502-4545
