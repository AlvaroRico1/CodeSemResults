dse_step6 (void)
{
  basic_block bb;

  FOR_ALL_BB_FN (bb, cfun)
    {
      bb_info_t bb_info = bb_table[bb->index];
      insn_info_t insn_info = bb_info->last_insn;

      while (insn_info)
	{
	  /* There may have been code deleted by the dce pass run before
	     this phase.  */
	  if (insn_info->insn
	      && INSN_P (insn_info->insn)
	      && !insn_info->cannot_delete)
	    {
	      store_info *s_info = insn_info->store_rec;

	      while (s_info && !s_info->is_set)
		s_info = s_info->next;
	      if (s_info
		  && s_info->redundant_reason
		  && s_info->redundant_reason->insn
		  && INSN_P (s_info->redundant_reason->insn))
		{
		  rtx_insn *rinsn = s_info->redundant_reason->insn;
		  if (dump_file && (dump_flags & TDF_DETAILS))
		    fprintf (dump_file, "Locally deleting insn %d "
					"because insn %d stores the "
					"same value and couldn't be "
					"eliminated\n",
					INSN_UID (insn_info->insn),
					INSN_UID (rinsn));
		  delete_dead_store_insn (insn_info);
		}


// Source: dse.c
// Lines 3572-3607
