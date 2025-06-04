scan_reads (insn_info_t insn_info, bitmap gen, bitmap kill)
{
  read_info_t read_info = insn_info->read_rec;
  int i;
  group_info *group;

  /* If this insn reads the frame, kill all the frame related stores.  */
  if (insn_info->frame_read)
    {
      FOR_EACH_VEC_ELT (rtx_group_vec, i, group)
	if (group->process_globally && group->frame_related)
	  {
	    if (kill)
	      bitmap_ior_into (kill, group->group_kill);
	    bitmap_and_compl_into (gen, group->group_kill);
	  }
    }
  if (insn_info->non_frame_wild_read)
    {
      /* Kill all non-frame related stores.  Kill all stores of variables that
         escape.  */
      if (kill)
        bitmap_ior_into (kill, kill_on_calls);
      bitmap_and_compl_into (gen, kill_on_calls);
      FOR_EACH_VEC_ELT (rtx_group_vec, i, group)
	if (group->process_globally && !group->frame_related)
	  {
	    if (kill)
	      bitmap_ior_into (kill, group->group_kill);
	    bitmap_and_compl_into (gen, group->group_kill);
	  }
    }
  while (read_info)
    {
      FOR_EACH_VEC_ELT (rtx_group_vec, i, group)
	{
	  if (group->process_globally)
	    {
	      if (i == read_info->group_id)
		{
		  HOST_WIDE_INT offset, width;
		  /* Reads with non-constant size kill all DSE opportunities
		     in the group.  */
		  if (!read_info->offset.is_constant (&offset)
		      || !read_info->width.is_constant (&width)
		      || !known_size_p (width))
		    {
		      /* Handle block mode reads.  */
		      if (kill)
			bitmap_ior_into (kill, group->group_kill);
		      bitmap_and_compl_into (gen, group->group_kill);
		    }
		  else
		    {
		      /* The groups are the same, just process the
			 offsets.  */
		      HOST_WIDE_INT j;
		      HOST_WIDE_INT end = offset + width;
		      for (j = offset; j < end; j++)
			{
			  int index = get_bitmap_index (group, j);
			  if (index != 0)
			    {
			      if (kill)
				bitmap_set_bit (kill, index);
			      bitmap_clear_bit (gen, index);
			    }
			}
		    }
		}
	      else
		{
		  /* The groups are different, if the alias sets
		     conflict, clear the entire group.  We only need
		     to apply this test if the read_info is a cselib
		     read.  Anything with a constant base cannot alias
		     something else with a different constant
		     base.  */
		  if ((read_info->group_id < 0)
		      && canon_true_dependence (group->base_mem,
						GET_MODE (group->base_mem),
						group->canon_base_addr,
						read_info->mem, NULL_RTX))
		    {
		      if (kill)
			bitmap_ior_into (kill, group->group_kill);
		      bitmap_and_compl_into (gen, group->group_kill);
		    }
		}
	    }
	}

      read_info = read_info->next;
    }
}


// Source: dse.c
// Lines 3014-3108
