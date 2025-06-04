find_moveable_pseudos (void)
{
  unsigned i;
  int max_regs = max_reg_num ();
  int max_uid = get_max_uid ();
  basic_block bb;
  int *uid_luid = XNEWVEC (int, max_uid);
  rtx_insn **closest_uses = XNEWVEC (rtx_insn *, max_regs);
  /* A set of registers which are live but not modified throughout a block.  */
  bitmap_head *bb_transp_live = XNEWVEC (bitmap_head,
					 last_basic_block_for_fn (cfun));
  /* A set of registers which only exist in a given basic block.  */
  bitmap_head *bb_local = XNEWVEC (bitmap_head,
				   last_basic_block_for_fn (cfun));
  /* A set of registers which are set once, in an instruction that can be
     moved freely downwards, but are otherwise transparent to a block.  */
  bitmap_head *bb_moveable_reg_sets = XNEWVEC (bitmap_head,
					       last_basic_block_for_fn (cfun));
  auto_bitmap live, used, set, interesting, unusable_as_input;
  bitmap_iterator bi;

  first_moveable_pseudo = max_regs;
  pseudo_replaced_reg.release ();
  pseudo_replaced_reg.safe_grow_cleared (max_regs);

  df_analyze ();
  calculate_dominance_info (CDI_DOMINATORS);

  i = 0;
  FOR_EACH_BB_FN (bb, cfun)
    {
      rtx_insn *insn;
      bitmap transp = bb_transp_live + bb->index;
      bitmap moveable = bb_moveable_reg_sets + bb->index;
      bitmap local = bb_local + bb->index;

      bitmap_initialize (local, 0);
      bitmap_initialize (transp, 0);
      bitmap_initialize (moveable, 0);
      bitmap_copy (live, df_get_live_out (bb));
      bitmap_and_into (live, df_get_live_in (bb));
      bitmap_copy (transp, live);
      bitmap_clear (moveable);
      bitmap_clear (live);
      bitmap_clear (used);
      bitmap_clear (set);
      FOR_BB_INSNS (bb, insn)
	if (NONDEBUG_INSN_P (insn))
	  {
	    df_insn_info *insn_info = DF_INSN_INFO_GET (insn);
	    df_ref def, use;

	    uid_luid[INSN_UID (insn)] = i++;
	    
	    def = df_single_def (insn_info);
	    use = df_single_use (insn_info);
	    if (use
		&& def
		&& DF_REF_REGNO (use) == DF_REF_REGNO (def)
		&& !bitmap_bit_p (set, DF_REF_REGNO (use))
		&& rtx_moveable_p (&PATTERN (insn), OP_IN))
	      {
		unsigned regno = DF_REF_REGNO (use);
		bitmap_set_bit (moveable, regno);
		bitmap_set_bit (set, regno);
		bitmap_set_bit (used, regno);
		bitmap_clear_bit (transp, regno);
		continue;
	      }
	    FOR_EACH_INSN_INFO_USE (use, insn_info)
	      {
		unsigned regno = DF_REF_REGNO (use);
		bitmap_set_bit (used, regno);
		if (bitmap_clear_bit (moveable, regno))
		  bitmap_clear_bit (transp, regno);
	      }

	    FOR_EACH_INSN_INFO_DEF (def, insn_info)
	      {
		unsigned regno = DF_REF_REGNO (def);
		bitmap_set_bit (set, regno);
		bitmap_clear_bit (transp, regno);
		bitmap_clear_bit (moveable, regno);
	      }
	  }
    }

  FOR_EACH_BB_FN (bb, cfun)
    {
      bitmap local = bb_local + bb->index;
      rtx_insn *insn;

      FOR_BB_INSNS (bb, insn)
	if (NONDEBUG_INSN_P (insn))
	  {
	    df_insn_info *insn_info = DF_INSN_INFO_GET (insn);
	    rtx_insn *def_insn;
	    rtx closest_use, note;
	    df_ref def, use;
	    unsigned regno;
	    bool all_dominated, all_local;
	    machine_mode mode;

	    def = df_single_def (insn_info);
	    /* There must be exactly one def in this insn.  */
	    if (!def || !single_set (insn))
	      continue;
	    /* This must be the only definition of the reg.  We also limit
	       which modes we deal with so that we can assume we can generate
	       move instructions.  */
	    regno = DF_REF_REGNO (def);
	    mode = GET_MODE (DF_REF_REG (def));
	    if (DF_REG_DEF_COUNT (regno) != 1
		|| !DF_REF_INSN_INFO (def)
		|| HARD_REGISTER_NUM_P (regno)
		|| DF_REG_EQ_USE_COUNT (regno) > 0
		|| (!INTEGRAL_MODE_P (mode) && !FLOAT_MODE_P (mode)))
	      continue;
	    def_insn = DF_REF_INSN (def);

	    for (note = REG_NOTES (def_insn); note; note = XEXP (note, 1))
	      if (REG_NOTE_KIND (note) == REG_EQUIV && MEM_P (XEXP (note, 0)))
		break;
		
	    if (note)
	      {
		if (dump_file)
		  fprintf (dump_file, "Ignoring reg %d, has equiv memory\n",
			   regno);
		bitmap_set_bit (unusable_as_input, regno);
		continue;
	      }

	    use = DF_REG_USE_CHAIN (regno);
	    all_dominated = true;
	    all_local = true;
	    closest_use = NULL_RTX;
	    for (; use; use = DF_REF_NEXT_REG (use))
	      {
		rtx_insn *insn;
		if (!DF_REF_INSN_INFO (use))
		  {
		    all_dominated = false;
		    all_local = false;
		    break;
		  }
		insn = DF_REF_INSN (use);
		if (DEBUG_INSN_P (insn))
		  continue;
		if (BLOCK_FOR_INSN (insn) != BLOCK_FOR_INSN (def_insn))
		  all_local = false;
		if (!insn_dominated_by_p (insn, def_insn, uid_luid))
		  all_dominated = false;
		if (closest_use != insn && closest_use != const0_rtx)
		  {
		    if (closest_use == NULL_RTX)
		      closest_use = insn;
		    else if (insn_dominated_by_p (closest_use, insn, uid_luid))
		      closest_use = insn;
		    else if (!insn_dominated_by_p (insn, closest_use, uid_luid))
		      closest_use = const0_rtx;
		  }
	      }
	    if (!all_dominated)
	      {
		if (dump_file)
		  fprintf (dump_file, "Reg %d not all uses dominated by set\n",
			   regno);
		continue;
	      }
	    if (all_local)
	      bitmap_set_bit (local, regno);
	    if (closest_use == const0_rtx || closest_use == NULL
		|| next_nonnote_nondebug_insn (def_insn) == closest_use)
	      {
		if (dump_file)
		  fprintf (dump_file, "Reg %d uninteresting%s\n", regno,
			   closest_use == const0_rtx || closest_use == NULL
			   ? " (no unique first use)" : "");
		continue;
	      }
	    if (HAVE_cc0 && reg_referenced_p (cc0_rtx, PATTERN (closest_use)))
	      {
		if (dump_file)
		  fprintf (dump_file, "Reg %d: closest user uses cc0\n",
			   regno);
		continue;
	      }

	    bitmap_set_bit (interesting, regno);
	    /* If we get here, we know closest_use is a non-NULL insn
	       (as opposed to const_0_rtx).  */
	    closest_uses[regno] = as_a <rtx_insn *> (closest_use);

	    if (dump_file && (all_local || all_dominated))
	      {
		fprintf (dump_file, "Reg %u:", regno);
		if (all_local)
		  fprintf (dump_file, " local to bb %d", bb->index);
		if (all_dominated)
		  fprintf (dump_file, " def dominates all uses");
		if (closest_use != const0_rtx)
		  fprintf (dump_file, " has unique first use");
		fputs ("\n", dump_file);
	      }
	  }
    }

  EXECUTE_IF_SET_IN_BITMAP (interesting, 0, i, bi)
    {
      df_ref def = DF_REG_DEF_CHAIN (i);
      rtx_insn *def_insn = DF_REF_INSN (def);
      basic_block def_block = BLOCK_FOR_INSN (def_insn);
      bitmap def_bb_local = bb_local + def_block->index;
      bitmap def_bb_moveable = bb_moveable_reg_sets + def_block->index;
      bitmap def_bb_transp = bb_transp_live + def_block->index;
      bool local_to_bb_p = bitmap_bit_p (def_bb_local, i);
      rtx_insn *use_insn = closest_uses[i];
      df_ref use;
      bool all_ok = true;
      bool all_transp = true;

      if (!REG_P (DF_REF_REG (def)))
	continue;

      if (!local_to_bb_p)
	{
	  if (dump_file)
	    fprintf (dump_file, "Reg %u not local to one basic block\n",
		     i);
	  continue;
	}
      if (reg_equiv_init (i) != NULL_RTX)
	{
	  if (dump_file)
	    fprintf (dump_file, "Ignoring reg %u with equiv init insn\n",
		     i);
	  continue;
	}
      if (!rtx_moveable_p (&PATTERN (def_insn), OP_IN))
	{
	  if (dump_file)
	    fprintf (dump_file, "Found def insn %d for %d to be not moveable\n",
		     INSN_UID (def_insn), i);
	  continue;
	}
      if (dump_file)
	fprintf (dump_file, "Examining insn %d, def for %d\n",
		 INSN_UID (def_insn), i);
      FOR_EACH_INSN_USE (use, def_insn)
	{
	  unsigned regno = DF_REF_REGNO (use);
	  if (bitmap_bit_p (unusable_as_input, regno))
	    {
	      all_ok = false;
	      if (dump_file)
		fprintf (dump_file, "  found unusable input reg %u.\n", regno);
	      break;
	    }
	  if (!bitmap_bit_p (def_bb_transp, regno))
	    {
	      if (bitmap_bit_p (def_bb_moveable, regno)
		  && !control_flow_insn_p (use_insn)
		  && (!HAVE_cc0 || !sets_cc0_p (use_insn)))
		{
		  if (modified_between_p (DF_REF_REG (use), def_insn, use_insn))
		    {
		      rtx_insn *x = NEXT_INSN (def_insn);
		      while (!modified_in_p (DF_REF_REG (use), x))
			{
			  gcc_assert (x != use_insn);
			  x = NEXT_INSN (x);
			}
		      if (dump_file)
			fprintf (dump_file, "  input reg %u modified but insn %d moveable\n",
				 regno, INSN_UID (x));
		      emit_insn_after (PATTERN (x), use_insn);
		      set_insn_deleted (x);
		    }
		  else
		    {
		      if (dump_file)
			fprintf (dump_file, "  input reg %u modified between def and use\n",
				 regno);
		      all_transp = false;
		    }
		}
	      else
		all_transp = false;
	    }
	}
      if (!all_ok)
	continue;
      if (!dbg_cnt (ira_move))
	break;
      if (dump_file)
	fprintf (dump_file, "  all ok%s\n", all_transp ? " and transp" : "");

      if (all_transp)
	{
	  rtx def_reg = DF_REF_REG (def);
	  rtx newreg = ira_create_new_reg (def_reg);
	  if (validate_change (def_insn, DF_REF_REAL_LOC (def), newreg, 0))
	    {
	      unsigned nregno = REGNO (newreg);
	      emit_insn_before (gen_move_insn (def_reg, newreg), use_insn);
	      nregno -= max_regs;
	      pseudo_replaced_reg[nregno] = def_reg;
	    }
	}
    }


// Source: ira.c
// Lines 4548-4858
