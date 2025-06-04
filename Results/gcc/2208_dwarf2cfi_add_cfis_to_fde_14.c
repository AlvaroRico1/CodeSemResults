add_cfis_to_fde (void)
{
  dw_fde_ref fde = cfun->fde;
  rtx_insn *insn, *next;

  for (insn = get_insns (); insn; insn = next)
    {
      next = NEXT_INSN (insn);

      if (NOTE_P (insn) && NOTE_KIND (insn) == NOTE_INSN_SWITCH_TEXT_SECTIONS)
	fde->dw_fde_switch_cfi_index = vec_safe_length (fde->dw_fde_cfi);

      if (NOTE_P (insn) && NOTE_KIND (insn) == NOTE_INSN_CFI)
	{
	  bool required = cfi_label_required_p (NOTE_CFI (insn));
	  while (next)
	    if (NOTE_P (next) && NOTE_KIND (next) == NOTE_INSN_CFI)
	      {
		required |= cfi_label_required_p (NOTE_CFI (next));
		next = NEXT_INSN (next);
	      }
	    else if (active_insn_p (next)
		     || (NOTE_P (next) && (NOTE_KIND (next)
					   == NOTE_INSN_SWITCH_TEXT_SECTIONS)))
	      break;
	    else
	      next = NEXT_INSN (next);
	  if (required)
	    {
	      int num = dwarf2out_cfi_label_num;
	      const char *label = dwarf2out_cfi_label ();
	      dw_cfi_ref xcfi;

	      /* Set the location counter to the new label.  */
	      xcfi = new_cfi ();
	      xcfi->dw_cfi_opc = DW_CFA_advance_loc4;
	      xcfi->dw_cfi_oprnd1.dw_cfi_addr = label;
	      vec_safe_push (fde->dw_fde_cfi, xcfi);

	      rtx_note *tmp = emit_note_before (NOTE_INSN_CFI_LABEL, insn);
	      NOTE_LABEL_NUMBER (tmp) = num;
	    }

	  do
	    {
	      if (NOTE_P (insn) && NOTE_KIND (insn) == NOTE_INSN_CFI)
		vec_safe_push (fde->dw_fde_cfi, NOTE_CFI (insn));
	      insn = NEXT_INSN (insn);
	    }
	  while (insn != next);
	}
    }
}


// Source: dwarf2cfi.c
// Lines 2316-2368
