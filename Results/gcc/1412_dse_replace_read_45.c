replace_read (store_info *store_info, insn_info_t store_insn,
	      read_info_t read_info, insn_info_t read_insn, rtx *loc,
	      bitmap regs_live)
{
  machine_mode store_mode = GET_MODE (store_info->mem);
  machine_mode read_mode = GET_MODE (read_info->mem);
  rtx_insn *insns, *this_insn;
  rtx read_reg;
  basic_block bb;

  if (!dbg_cnt (dse))
    return false;

  /* Create a sequence of instructions to set up the read register.
     This sequence goes immediately before the store and its result
     is read by the load.

     We need to keep this in perspective.  We are replacing a read
     with a sequence of insns, but the read will almost certainly be
     in cache, so it is not going to be an expensive one.  Thus, we
     are not willing to do a multi insn shift or worse a subroutine
     call to get rid of the read.  */
  if (dump_file && (dump_flags & TDF_DETAILS))
    fprintf (dump_file, "trying to replace %smode load in insn %d"
	     " from %smode store in insn %d\n",
	     GET_MODE_NAME (read_mode), INSN_UID (read_insn->insn),
	     GET_MODE_NAME (store_mode), INSN_UID (store_insn->insn));
  start_sequence ();
  bb = BLOCK_FOR_INSN (read_insn->insn);
  read_reg = get_stored_val (store_info,
			     read_mode, read_info->offset, read_info->width,
			     bb, false);
  if (read_reg == NULL_RTX)
    {
      end_sequence ();
      if (dump_file && (dump_flags & TDF_DETAILS))
	fprintf (dump_file, " -- could not extract bits of stored value\n");
      return false;
    }
  /* Force the value into a new register so that it won't be clobbered
     between the store and the load.  */
  read_reg = copy_to_mode_reg (read_mode, read_reg);
  insns = get_insns ();
  end_sequence ();

  if (insns != NULL_RTX)
    {
      /* Now we have to scan the set of new instructions to see if the
	 sequence contains and sets of hardregs that happened to be
	 live at this point.  For instance, this can happen if one of
	 the insns sets the CC and the CC happened to be live at that
	 point.  This does occasionally happen, see PR 37922.  */
      bitmap regs_set = BITMAP_ALLOC (&reg_obstack);

      for (this_insn = insns;
	   this_insn != NULL_RTX; this_insn = NEXT_INSN (this_insn))
	{
	  if (insn_invalid_p (this_insn, false))
	    {
	      if (dump_file && (dump_flags & TDF_DETAILS))
		{
		  fprintf (dump_file, " -- replacing the loaded MEM with ");
		  print_simple_rtl (dump_file, read_reg);
		  fprintf (dump_file, " led to an invalid instruction\n");
		}
	      BITMAP_FREE (regs_set);
	      return false;
	    }
	  note_stores (this_insn, look_for_hardregs, regs_set);
	}

      bitmap_and_into (regs_set, regs_live);
      if (!bitmap_empty_p (regs_set))
	{
	  if (dump_file && (dump_flags & TDF_DETAILS))
	    {
	      fprintf (dump_file, "abandoning replacement because sequence "
				  "clobbers live hardregs:");
	      df_print_regset (dump_file, regs_set);
	    }

	  BITMAP_FREE (regs_set);
	  return false;
	}
      BITMAP_FREE (regs_set);
    }

  subrtx_iterator::array_type array;
  FOR_EACH_SUBRTX (iter, array, *loc, NONCONST)
    {
      const_rtx x = *iter;
      if (GET_RTX_CLASS (GET_CODE (x)) == RTX_AUTOINC)
	{
	  if (dump_file && (dump_flags & TDF_DETAILS))
	    fprintf (dump_file, " -- replacing the MEM failed due to address "
				"side-effects\n");
	  return false;
	}
    }

  if (validate_change (read_insn->insn, loc, read_reg, 0))
    {
      deferred_change *change = deferred_change_pool.allocate ();

      /* Insert this right before the store insn where it will be safe
	 from later insns that might change it before the read.  */
      emit_insn_before (insns, store_insn->insn);

      /* And now for the kludge part: cselib croaks if you just
	 return at this point.  There are two reasons for this:

	 1) Cselib has an idea of how many pseudos there are and
	 that does not include the new ones we just added.

	 2) Cselib does not know about the move insn we added
	 above the store_info, and there is no way to tell it
	 about it, because it has "moved on".

	 Problem (1) is fixable with a certain amount of engineering.
	 Problem (2) is requires starting the bb from scratch.  This
	 could be expensive.

	 So we are just going to have to lie.  The move/extraction
	 insns are not really an issue, cselib did not see them.  But
	 the use of the new pseudo read_insn is a real problem because
	 cselib has not scanned this insn.  The way that we solve this
	 problem is that we are just going to put the mem back for now
	 and when we are finished with the block, we undo this.  We
	 keep a table of mems to get rid of.  At the end of the basic
	 block we can put them back.  */

      *loc = read_info->mem;
      change->next = deferred_change_list;
      deferred_change_list = change;
      change->loc = loc;
      change->reg = read_reg;

      /* Get rid of the read_info, from the point of view of the
	 rest of dse, play like this read never happened.  */
      read_insn->read_rec = read_info->next;
      read_info_type_pool.remove (read_info);
      if (dump_file && (dump_flags & TDF_DETAILS))
	{
	  fprintf (dump_file, " -- replaced the loaded MEM with ");
	  print_simple_rtl (dump_file, read_reg);
	  fprintf (dump_file, "\n");
	}
      return true;
    }


// Source: dse.c
// Lines 1952-2100
