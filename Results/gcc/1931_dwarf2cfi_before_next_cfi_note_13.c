before_next_cfi_note (rtx_insn *start)
{
  rtx_insn *prev = start;
  while (start)
    {
      if (NOTE_P (start) && NOTE_KIND (start) == NOTE_INSN_CFI)
	return prev;
      prev = start;
      start = NEXT_INSN (start);
    }
  gcc_unreachable ();
}


// Source: dwarf2cfi.c
// Lines 2790-2801
