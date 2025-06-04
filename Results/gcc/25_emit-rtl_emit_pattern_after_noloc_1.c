emit_pattern_after_noloc (rtx x, rtx_insn *after, basic_block bb,
			  rtx_insn *(*make_raw)(rtx))
{
  rtx_insn *last = after;

  gcc_assert (after);

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
      last = emit_insn_after_1 (as_a <rtx_insn *> (x), after, bb);
      break;

#ifdef ENABLE_RTL_CHECKING
    case SEQUENCE:
      gcc_unreachable ();
      break;
#endif

    default:
      last = (*make_raw) (x);
      add_insn_after (last, after, bb);
      break;
    }

  return last;
}


// Source: emit-rtl.c
// Lines 4658-4693
