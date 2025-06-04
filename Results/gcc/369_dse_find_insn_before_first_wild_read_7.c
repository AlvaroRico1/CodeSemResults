find_insn_before_first_wild_read (bb_info_t bb_info)
{
  insn_info_t insn_info = bb_info->last_insn;
  insn_info_t last_wild_read = NULL;

  while (insn_info)
    {
      if (insn_info->wild_read)
	{
	  last_wild_read = insn_info->prev_insn;
	  /* Block starts with wild read.  */
	  if (!last_wild_read)
	    return NULL;
	}

      insn_info = insn_info->prev_insn;
    }

  if (last_wild_read)
    return last_wild_read;
  else
    return bb_info->last_insn;
}


// Source: dse.c
// Lines 3115-3137
