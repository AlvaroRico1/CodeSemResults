unshare_all_rtl_again (rtx_insn *insn)
{
  rtx_insn *p;
  tree decl;

  for (p = insn; p; p = NEXT_INSN (p))
    if (INSN_P (p))
      {
	reset_used_flags (PATTERN (p));
	reset_used_flags (REG_NOTES (p));
	if (CALL_P (p))
	  reset_used_flags (CALL_INSN_FUNCTION_USAGE (p));
      }

  /* Make sure that virtual stack slots are not shared.  */
  set_used_decls (DECL_INITIAL (cfun->decl));

  /* Make sure that virtual parameters are not shared.  */
  for (decl = DECL_ARGUMENTS (cfun->decl); decl; decl = DECL_CHAIN (decl))
    set_used_flags (DECL_RTL (decl));

  rtx temp;
  unsigned int i;
  FOR_EACH_VEC_SAFE_ELT (stack_slot_list, i, temp)
    reset_used_flags (temp);

  unshare_all_rtl_1 (insn);
}


// Source: emit-rtl.c
// Lines 2775-2802
