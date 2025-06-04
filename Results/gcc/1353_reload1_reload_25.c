reload (rtx_insn *first, int global)
{
  int i, n;
  rtx_insn *insn;
  struct elim_table *ep;
  basic_block bb;
  bool inserted;

  /* Make sure even insns with volatile mem refs are recognizable.  */
  init_recog ();

  failure = 0;

  reload_firstobj = XOBNEWVAR (&reload_obstack, char, 0);

  /* Make sure that the last insn in the chain
     is not something that needs reloading.  */
  emit_note (NOTE_INSN_DELETED);

  /* Enable find_equiv_reg to distinguish insns made by reload.  */
  reload_first_uid = get_max_uid ();

  /* Initialize the secondary memory table.  */
  clear_secondary_mem ();

  /* We don't have a stack slot for any spill reg yet.  */
  memset (spill_stack_slot, 0, sizeof spill_stack_slot);
  memset (spill_stack_slot_width, 0, sizeof spill_stack_slot_width);

  /* Initialize the save area information for caller-save, in case some
     are needed.  */
  init_save_areas ();

  /* Compute which hard registers are now in use
     as homes for pseudo registers.
     This is done here rather than (eg) in global_alloc
     because this point is reached even if not optimizing.  */
  for (i = FIRST_PSEUDO_REGISTER; i < max_regno; i++)
    mark_home_live (i);

  /* A function that has a nonlocal label that can reach the exit
     block via non-exceptional paths must save all call-saved
     registers.  */
  if (cfun->has_nonlocal_label
      && has_nonexceptional_receiver ())
    crtl->saves_all_registers = 1;

  if (crtl->saves_all_registers)
    for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
      if (! crtl->abi->clobbers_full_reg_p (i)
	  && ! fixed_regs[i]
	  && ! LOCAL_REGNO (i))
	df_set_regs_ever_live (i, true);

  /* Find all the pseudo registers that didn't get hard regs
     but do have known equivalent constants or memory slots.
     These include parameters (known equivalent to parameter slots)
     and cse'd or loop-moved constant memory addresses.

     Record constant equivalents in reg_equiv_constant
     so they will be substituted by find_reloads.
     Record memory equivalents in reg_mem_equiv so they can
     be substituted eventually by altering the REG-rtx's.  */

  grow_reg_equivs ();
  reg_old_renumber = XCNEWVEC (short, max_regno);
  memcpy (reg_old_renumber, reg_renumber, max_regno * sizeof (short));
  pseudo_forbidden_regs = XNEWVEC (HARD_REG_SET, max_regno);
  pseudo_previous_regs = XCNEWVEC (HARD_REG_SET, max_regno);

  CLEAR_HARD_REG_SET (bad_spill_regs_global);

  init_eliminable_invariants (first, true);
  init_elim_table ();

  /* Alter each pseudo-reg rtx to contain its hard reg number.  Assign
     stack slots to the pseudos that lack hard regs or equivalents.
     Do not touch virtual registers.  */

  temp_pseudo_reg_arr = XNEWVEC (int, max_regno - LAST_VIRTUAL_REGISTER - 1);
  for (n = 0, i = LAST_VIRTUAL_REGISTER + 1; i < max_regno; i++)
    temp_pseudo_reg_arr[n++] = i;

  if (ira_conflicts_p)
    /* Ask IRA to order pseudo-registers for better stack slot
       sharing.  */
    ira_sort_regnos_for_alter_reg (temp_pseudo_reg_arr, n, reg_max_ref_mode);

  for (i = 0; i < n; i++)
    alter_reg (temp_pseudo_reg_arr[i], -1, false);

  /* If we have some registers we think can be eliminated, scan all insns to
     see if there is an insn that sets one of these registers to something
     other than itself plus a constant.  If so, the register cannot be
     eliminated.  Doing this scan here eliminates an extra pass through the
     main reload loop in the most common case where register elimination
     cannot be done.  */
  for (insn = first; insn && num_eliminable; insn = NEXT_INSN (insn))
    if (INSN_P (insn))
      note_pattern_stores (PATTERN (insn), mark_not_eliminable, NULL);

  maybe_fix_stack_asms ();

  insns_need_reload = 0;
  something_needs_elimination = 0;

  /* Initialize to -1, which means take the first spill register.  */
  last_spill_reg = -1;

  /* Spill any hard regs that we know we can't eliminate.  */
  CLEAR_HARD_REG_SET (used_spill_regs);
  /* There can be multiple ways to eliminate a register;
     they should be listed adjacently.
     Elimination for any register fails only if all possible ways fail.  */
  for (ep = reg_eliminate; ep < &reg_eliminate[NUM_ELIMINABLE_REGS]; )
    {
      int from = ep->from;
      int can_eliminate = 0;
      do
	{
          can_eliminate |= ep->can_eliminate;
          ep++;
	}
      while (ep < &reg_eliminate[NUM_ELIMINABLE_REGS] && ep->from == from);
      if (! can_eliminate)
	spill_hard_reg (from, 1);
    }

  if (!HARD_FRAME_POINTER_IS_FRAME_POINTER && frame_pointer_needed)
    spill_hard_reg (HARD_FRAME_POINTER_REGNUM, 1);

  finish_spills (global);

  /* From now on, we may need to generate moves differently.  We may also
     allow modifications of insns which cause them to not be recognized.
     Any such modifications will be cleaned up during reload itself.  */
  reload_in_progress = 1;

  /* This loop scans the entire function each go-round
     and repeats until one repetition spills no additional hard regs.  */
  for (;;)
    {
      int something_changed;
      poly_int64 starting_frame_size;

      starting_frame_size = get_frame_size ();
      something_was_spilled = false;

      set_initial_elim_offsets ();
      set_initial_label_offsets ();

      /* For each pseudo register that has an equivalent location defined,
	 try to eliminate any eliminable registers (such as the frame pointer)
	 assuming initial offsets for the replacement register, which
	 is the normal case.

	 If the resulting location is directly addressable, substitute
	 the MEM we just got directly for the old REG.

	 If it is not addressable but is a constant or the sum of a hard reg
	 and constant, it is probably not addressable because the constant is
	 out of range, in that case record the address; we will generate
	 hairy code to compute the address in a register each time it is
	 needed.  Similarly if it is a hard register, but one that is not
	 valid as an address register.

	 If the location is not addressable, but does not have one of the
	 above forms, assign a stack slot.  We have to do this to avoid the
	 potential of producing lots of reloads if, e.g., a location involves
	 a pseudo that didn't get a hard register and has an equivalent memory
	 location that also involves a pseudo that didn't get a hard register.

	 Perhaps at some point we will improve reload_when_needed handling
	 so this problem goes away.  But that's very hairy.  */

      for (i = FIRST_PSEUDO_REGISTER; i < max_regno; i++)
	if (reg_renumber[i] < 0 && reg_equiv_memory_loc (i))
	  {
	    rtx x = eliminate_regs (reg_equiv_memory_loc (i), VOIDmode,
				    NULL_RTX);

	    if (strict_memory_address_addr_space_p
		  (GET_MODE (regno_reg_rtx[i]), XEXP (x, 0),
		   MEM_ADDR_SPACE (x)))
	      reg_equiv_mem (i) = x, reg_equiv_address (i) = 0;
	    else if (CONSTANT_P (XEXP (x, 0))
		     || (REG_P (XEXP (x, 0))
			 && REGNO (XEXP (x, 0)) < FIRST_PSEUDO_REGISTER)
		     || (GET_CODE (XEXP (x, 0)) == PLUS
			 && REG_P (XEXP (XEXP (x, 0), 0))
			 && (REGNO (XEXP (XEXP (x, 0), 0))
			     < FIRST_PSEUDO_REGISTER)
			 && CONSTANT_P (XEXP (XEXP (x, 0), 1))))
	      reg_equiv_address (i) = XEXP (x, 0), reg_equiv_mem (i) = 0;
	    else
	      {
		/* Make a new stack slot.  Then indicate that something
		   changed so we go back and recompute offsets for
		   eliminable registers because the allocation of memory
		   below might change some offset.  reg_equiv_{mem,address}
		   will be set up for this pseudo on the next pass around
		   the loop.  */
		reg_equiv_memory_loc (i) = 0;
		reg_equiv_init (i) = 0;
		alter_reg (i, -1, true);
	      }
	  }

      if (caller_save_needed)
	setup_save_areas ();

      if (maybe_ne (starting_frame_size, 0) && crtl->stack_alignment_needed)
	{
	  /* If we have a stack frame, we must align it now.  The
	     stack size may be a part of the offset computation for
	     register elimination.  So if this changes the stack size,
	     then repeat the elimination bookkeeping.  We don't
	     realign when there is no stack, as that will cause a
	     stack frame when none is needed should
	     TARGET_STARTING_FRAME_OFFSET not be already aligned to
	     STACK_BOUNDARY.  */
	  assign_stack_local (BLKmode, 0, crtl->stack_alignment_needed);
	}
      /* If we allocated another stack slot, redo elimination bookkeeping.  */
      if (something_was_spilled
	  || maybe_ne (starting_frame_size, get_frame_size ()))
	{
	  if (update_eliminables_and_spill ())
	    finish_spills (0);
	  continue;
	}

      if (caller_save_needed)
	{
	  save_call_clobbered_regs ();
	  /* That might have allocated new insn_chain structures.  */
	  reload_firstobj = XOBNEWVAR (&reload_obstack, char, 0);
	}

      calculate_needs_all_insns (global);

      if (! ira_conflicts_p)
	/* Don't do it for IRA.  We need this info because we don't
	   change live_throughout and dead_or_set for chains when IRA
	   is used.  */
	CLEAR_REG_SET (&spilled_pseudos);

      something_changed = 0;

      /* If we allocated any new memory locations, make another pass
	 since it might have changed elimination offsets.  */
      if (something_was_spilled
	  || maybe_ne (starting_frame_size, get_frame_size ()))
	something_changed = 1;

      /* Even if the frame size remained the same, we might still have
	 changed elimination offsets, e.g. if find_reloads called
	 force_const_mem requiring the back end to allocate a constant
	 pool base register that needs to be saved on the stack.  */
      else if (!verify_initial_elim_offsets ())
	something_changed = 1;

      if (update_eliminables_and_spill ())
	{
	  finish_spills (0);
	  something_changed = 1;
	}
      else
	{
	  select_reload_regs ();
	  if (failure)
	    goto failed;
	  if (insns_need_reload)
	    something_changed |= finish_spills (global);
	}

      if (! something_changed)
	break;

      if (caller_save_needed)
	delete_caller_save_insns ();

      obstack_free (&reload_obstack, reload_firstobj);
    }

  /* If global-alloc was run, notify it of any register eliminations we have
     done.  */
  if (global)
    for (ep = reg_eliminate; ep < &reg_eliminate[NUM_ELIMINABLE_REGS]; ep++)
      if (ep->can_eliminate)
	mark_elimination (ep->from, ep->to);

  remove_init_insns ();

  /* Use the reload registers where necessary
     by generating move instructions to move the must-be-register
     values into or out of the reload registers.  */

  if (insns_need_reload != 0 || something_needs_elimination
      || something_needs_operands_changed)
    {
      poly_int64 old_frame_size = get_frame_size ();

      reload_as_needed (global);

      gcc_assert (known_eq (old_frame_size, get_frame_size ()));

      gcc_assert (verify_initial_elim_offsets ());
    }

  /* If we were able to eliminate the frame pointer, show that it is no
     longer live at the start of any basic block.  If it ls live by
     virtue of being in a pseudo, that pseudo will be marked live
     and hence the frame pointer will be known to be live via that
     pseudo.  */

  if (! frame_pointer_needed)
    FOR_EACH_BB_FN (bb, cfun)
      bitmap_clear_bit (df_get_live_in (bb), HARD_FRAME_POINTER_REGNUM);

  /* Come here (with failure set nonzero) if we can't get enough spill
     regs.  */
 failed:

  CLEAR_REG_SET (&changed_allocation_pseudos);
  CLEAR_REG_SET (&spilled_pseudos);
  reload_in_progress = 0;

  /* Now eliminate all pseudo regs by modifying them into
     their equivalent memory references.
     The REG-rtx's for the pseudos are modified in place,
     so all insns that used to refer to them now refer to memory.

     For a reg that has a reg_equiv_address, all those insns
     were changed by reloading so that no insns refer to it any longer;
     but the DECL_RTL of a variable decl may refer to it,
     and if so this causes the debugging info to mention the variable.  */

  for (i = FIRST_PSEUDO_REGISTER; i < max_regno; i++)
    {
      rtx addr = 0;

      if (reg_equiv_mem (i))
	addr = XEXP (reg_equiv_mem (i), 0);

      if (reg_equiv_address (i))
	addr = reg_equiv_address (i);

      if (addr)
	{
	  if (reg_renumber[i] < 0)
	    {
	      rtx reg = regno_reg_rtx[i];

	      REG_USERVAR_P (reg) = 0;
	      PUT_CODE (reg, MEM);
	      XEXP (reg, 0) = addr;
	      if (reg_equiv_memory_loc (i))
		MEM_COPY_ATTRIBUTES (reg, reg_equiv_memory_loc (i));
	      else
		MEM_ATTRS (reg) = 0;
	      MEM_NOTRAP_P (reg) = 1;
	    }
	  else if (reg_equiv_mem (i))
	    XEXP (reg_equiv_mem (i), 0) = addr;
	}

      /* We don't want complex addressing modes in debug insns
	 if simpler ones will do, so delegitimize equivalences
	 in debug insns.  */
      if (MAY_HAVE_DEBUG_BIND_INSNS && reg_renumber[i] < 0)
	{
	  rtx reg = regno_reg_rtx[i];
	  rtx equiv = 0;
	  df_ref use, next;

	  if (reg_equiv_constant (i))
	    equiv = reg_equiv_constant (i);
	  else if (reg_equiv_invariant (i))
	    equiv = reg_equiv_invariant (i);
	  else if (reg && MEM_P (reg))
	    equiv = targetm.delegitimize_address (reg);
	  else if (reg && REG_P (reg) && (int)REGNO (reg) != i)
	    equiv = reg;

	  if (equiv == reg)
	    continue;

	  for (use = DF_REG_USE_CHAIN (i); use; use = next)
	    {
	      insn = DF_REF_INSN (use);

	      /* Make sure the next ref is for a different instruction,
		 so that we're not affected by the rescan.  */
	      next = DF_REF_NEXT_REG (use);
	      while (next && DF_REF_INSN (next) == insn)
		next = DF_REF_NEXT_REG (next);

	      if (DEBUG_BIND_INSN_P (insn))
		{
		  if (!equiv)
		    {
		      INSN_VAR_LOCATION_LOC (insn) = gen_rtx_UNKNOWN_VAR_LOC ();
		      df_insn_rescan_debug_internal (insn);
		    }
		  else
		    INSN_VAR_LOCATION_LOC (insn)
		      = simplify_replace_rtx (INSN_VAR_LOCATION_LOC (insn),
					      reg, equiv);
		}
	    }
	}
    }

  /* We must set reload_completed now since the cleanup_subreg_operands call
     below will re-recognize each insn and reload may have generated insns
     which are only valid during and after reload.  */
  reload_completed = 1;

  /* Make a pass over all the insns and delete all USEs which we inserted
     only to tag a REG_EQUAL note on them.  Remove all REG_DEAD and REG_UNUSED
     notes.  Delete all CLOBBER insns, except those that refer to the return
     value and the special mem:BLK CLOBBERs added to prevent the scheduler
     from misarranging variable-array code, and simplify (subreg (reg))
     operands.  Strip and regenerate REG_INC notes that may have been moved
     around.  */

  for (insn = first; insn; insn = NEXT_INSN (insn))
    if (INSN_P (insn))
      {
	rtx *pnote;

	if (CALL_P (insn))
	  replace_pseudos_in (& CALL_INSN_FUNCTION_USAGE (insn),
			      VOIDmode, CALL_INSN_FUNCTION_USAGE (insn));

	if ((GET_CODE (PATTERN (insn)) == USE
	     /* We mark with QImode USEs introduced by reload itself.  */
	     && (GET_MODE (insn) == QImode
		 || find_reg_note (insn, REG_EQUAL, NULL_RTX)))
	    || (GET_CODE (PATTERN (insn)) == CLOBBER
		&& (!MEM_P (XEXP (PATTERN (insn), 0))
		    || GET_MODE (XEXP (PATTERN (insn), 0)) != BLKmode
		    || (GET_CODE (XEXP (XEXP (PATTERN (insn), 0), 0)) != SCRATCH
			&& XEXP (XEXP (PATTERN (insn), 0), 0)
				!= stack_pointer_rtx))
		&& (!REG_P (XEXP (PATTERN (insn), 0))
		    || ! REG_FUNCTION_VALUE_P (XEXP (PATTERN (insn), 0)))))
	  {
	    delete_insn (insn);
	    continue;
	  }

	/* Some CLOBBERs may survive until here and still reference unassigned
	   pseudos with const equivalent, which may in turn cause ICE in later
	   passes if the reference remains in place.  */
	if (GET_CODE (PATTERN (insn)) == CLOBBER)
	  replace_pseudos_in (& XEXP (PATTERN (insn), 0),
			      VOIDmode, PATTERN (insn));

	/* Discard obvious no-ops, even without -O.  This optimization
	   is fast and doesn't interfere with debugging.  */
	if (NONJUMP_INSN_P (insn)
	    && GET_CODE (PATTERN (insn)) == SET
	    && REG_P (SET_SRC (PATTERN (insn)))
	    && REG_P (SET_DEST (PATTERN (insn)))
	    && (REGNO (SET_SRC (PATTERN (insn)))
		== REGNO (SET_DEST (PATTERN (insn)))))
	  {
	    delete_insn (insn);
	    continue;
	  }

	pnote = &REG_NOTES (insn);
	while (*pnote != 0)
	  {
	    if (REG_NOTE_KIND (*pnote) == REG_DEAD
		|| REG_NOTE_KIND (*pnote) == REG_UNUSED
		|| REG_NOTE_KIND (*pnote) == REG_INC)
	      *pnote = XEXP (*pnote, 1);
	    else
	      pnote = &XEXP (*pnote, 1);
	  }

	if (AUTO_INC_DEC)
	  add_auto_inc_notes (insn, PATTERN (insn));

	/* Simplify (subreg (reg)) if it appears as an operand.  */
	cleanup_subreg_operands (insn);

	/* Clean up invalid ASMs so that they don't confuse later passes.
	   See PR 21299.  */
	if (asm_noperands (PATTERN (insn)) >= 0)
	  {
	    extract_insn (insn);
	    if (!constrain_operands (1, get_enabled_alternatives (insn)))
	      {
		error_for_asm (insn,
			       "%<asm%> operand has impossible constraints");
		delete_insn (insn);
		continue;
	      }
	  }
      }

  free (temp_pseudo_reg_arr);

  /* Indicate that we no longer have known memory locations or constants.  */
  free_reg_equiv ();

  free (reg_max_ref_mode);
  free (reg_old_renumber);
  free (pseudo_previous_regs);
  free (pseudo_forbidden_regs);

  CLEAR_HARD_REG_SET (used_spill_regs);
  for (i = 0; i < n_spills; i++)
    SET_HARD_REG_BIT (used_spill_regs, spill_regs[i]);

  /* Free all the insn_chain structures at once.  */
  obstack_free (&reload_obstack, reload_startobj);
  unused_insn_chains = 0;

  inserted = fixup_abnormal_edges ();

  /* We've possibly turned single trapping insn into multiple ones.  */
  if (cfun->can_throw_non_call_exceptions)
    {
      auto_sbitmap blocks (last_basic_block_for_fn (cfun));
      bitmap_ones (blocks);
      find_many_sub_basic_blocks (blocks);
    }

  if (inserted)
    commit_edge_insertions ();

  /* Replacing pseudos with their memory equivalents might have
     created shared rtx.  Subsequent passes would get confused
     by this, so unshare everything here.  */
  unshare_all_rtl_again (first);

#ifdef STACK_BOUNDARY
  /* init_emit has set the alignment of the hard frame pointer
     to STACK_BOUNDARY.  It is very likely no longer valid if
     the hard frame pointer was used for register allocation.  */
  if (!frame_pointer_needed)
    REGNO_POINTER_ALIGN (HARD_FRAME_POINTER_REGNUM) = BITS_PER_UNIT;
#endif

  substitute_stack.release ();

  gcc_assert (bitmap_empty_p (&spilled_pseudos));

  reload_completed = !failure;

  return need_dce;
}


// Source: reload1.c
// Lines 745-1301
