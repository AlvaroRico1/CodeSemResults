change_sp_offset (rtx_insn *insns, poly_int64 sp_offset)
{
  for (rtx_insn *insn = insns; insn != NULL; insn = NEXT_INSN (insn))
    eliminate_regs_in_insn (insn, false, false, sp_offset);
}


// Source: lra-remat.c
// Lines 1005-1009
