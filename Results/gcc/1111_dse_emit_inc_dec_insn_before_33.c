emit_inc_dec_insn_before (rtx mem ATTRIBUTE_UNUSED,
			  rtx op ATTRIBUTE_UNUSED,
			  rtx dest, rtx src, rtx srcoff, void *arg)
{
  insn_info_t insn_info = (insn_info_t) arg;
  rtx_insn *insn = insn_info->insn, *new_insn, *cur;
  note_add_store_info info;

  /* We can reuse all operands without copying, because we are about
     to delete the insn that contained it.  */
  if (srcoff)
    {
      start_sequence ();
      emit_insn (gen_add3_insn (dest, src, srcoff));
      new_insn = get_insns ();
      end_sequence ();
    }
  else
    new_insn = gen_move_insn (dest, src);
  info.first = new_insn;
  info.fixed_regs_live = insn_info->fixed_regs_live;
  info.failure = false;
  for (cur = new_insn; cur; cur = NEXT_INSN (cur))
    {
      info.current = cur;
      note_stores (cur, note_add_store, &info);
    }

  /* If a failure was flagged above, return 1 so that for_each_inc_dec will
     return it immediately, communicating the failure to its caller.  */
  if (info.failure)
    return 1;

  emit_insn_before (new_insn, insn);

  return 0;
}


// Source: dse.c
// Lines 799-835
