get_trace_info (rtx_insn *insn)
{
  dw_trace_info dummy;
  dummy.head = insn;
  return trace_index->find_with_hash (&dummy, INSN_UID (insn));
}


// Source: dwarf2cfi.c
// Lines 365-370
