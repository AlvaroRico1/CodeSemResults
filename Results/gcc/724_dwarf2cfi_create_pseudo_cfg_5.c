create_pseudo_cfg (void)
{
  bool saw_barrier, switch_sections;
  dw_trace_info ti;
  rtx_insn *insn;
  unsigned i;

  /* The first trace begins at the start of the function,
     and begins with the CIE row state.  */
  trace_info.create (16);
  memset (&ti, 0, sizeof (ti));
  ti.head = get_insns ();
  ti.beg_row = cie_cfi_row;
  ti.cfa_store = cie_cfi_row->cfa;
  ti.cfa_temp.reg = INVALID_REGNUM;
  trace_info.quick_push (ti);

  if (cie_return_save)
    ti.regs_saved_in_regs.safe_push (*cie_return_save);

  /* Walk all the insns, collecting start of trace locations.  */
  saw_barrier = false;
  switch_sections = false;
  for (insn = get_insns (); insn; insn = NEXT_INSN (insn))
    {
      if (BARRIER_P (insn))
	saw_barrier = true;
      else if (NOTE_P (insn)
	       && NOTE_KIND (insn) == NOTE_INSN_SWITCH_TEXT_SECTIONS)
	{
	  /* We should have just seen a barrier.  */
	  gcc_assert (saw_barrier);
	  switch_sections = true;
	}
      /* Watch out for save_point notes between basic blocks.
	 In particular, a note after a barrier.  Do not record these,
	 delaying trace creation until the label.  */
      else if (save_point_p (insn)
	       && (LABEL_P (insn) || !saw_barrier))
	{
	  memset (&ti, 0, sizeof (ti));
	  ti.head = insn;
	  ti.switch_sections = switch_sections;
	  ti.id = trace_info.length ();
	  trace_info.safe_push (ti);

	  saw_barrier = false;
	  switch_sections = false;
	}
    }

  /* Create the trace index after we've finished building trace_info,
     avoiding stale pointer problems due to reallocation.  */
  trace_index
    = new hash_table<trace_info_hasher> (trace_info.length ());
  dw_trace_info *tp;
  FOR_EACH_VEC_ELT (trace_info, i, tp)
    {
      dw_trace_info **slot;

      if (dump_file)
	fprintf (dump_file, "Creating trace %u : start at %s %d%s\n", tp->id,
		 rtx_name[(int) GET_CODE (tp->head)], INSN_UID (tp->head),
		 tp->switch_sections ? " (section switch)" : "");

      slot = trace_index->find_slot_with_hash (tp, INSN_UID (tp->head), INSERT);
      gcc_assert (*slot == NULL);
      *slot = tp;
    }
}


// Source: dwarf2cfi.c
// Lines 2941-3010
