mark_target_live_regs (rtx_insn *insns, rtx target_maybe_return, struct resources *res)
{
  int b = -1;
  unsigned int i;
  struct target_info *tinfo = NULL;
  rtx_insn *insn;
  rtx jump_target;
  HARD_REG_SET scratch;
  struct resources set, needed;

  /* Handle end of function.  */
  if (target_maybe_return == 0 || ANY_RETURN_P (target_maybe_return))
    {
      *res = end_of_function_needs;
      return;
    }

  /* We've handled the case of RETURN/SIMPLE_RETURN; we should now have an
     instruction.  */
  rtx_insn *target = as_a <rtx_insn *> (target_maybe_return);

  /* Handle return insn.  */
  if (return_insn_p (target))
    {
      *res = end_of_function_needs;
      mark_referenced_resources (target, res, false);
      return;
    }

  /* We have to assume memory is needed, but the CC isn't.  */
  res->memory = 1;
  res->volatil = 0;
  res->cc = 0;

  /* See if we have computed this value already.  */
  if (target_hash_table != NULL)
    {
      for (tinfo = target_hash_table[INSN_UID (target) % TARGET_HASH_PRIME];
	   tinfo; tinfo = tinfo->next)
	if (tinfo->uid == INSN_UID (target))
	  break;

      /* Start by getting the basic block number.  If we have saved
	 information, we can get it from there unless the insn at the
	 start of the basic block has been deleted.  */
      if (tinfo && tinfo->block != -1
	  && ! BB_HEAD (BASIC_BLOCK_FOR_FN (cfun, tinfo->block))->deleted ())
	b = tinfo->block;
    }

  if (b == -1)
    b = find_basic_block (target, param_max_delay_slot_live_search);

  if (target_hash_table != NULL)
    {
      if (tinfo)
	{
	  /* If the information is up-to-date, use it.  Otherwise, we will
	     update it below.  */
	  if (b == tinfo->block && b != -1 && tinfo->bb_tick == bb_ticks[b])
	    {
	      res->regs = tinfo->live_regs;
	      return;
	    }
	}
      else
	{
	  /* Allocate a place to put our results and chain it into the
	     hash table.  */
	  tinfo = XNEW (struct target_info);
	  tinfo->uid = INSN_UID (target);
	  tinfo->block = b;
	  tinfo->next
	    = target_hash_table[INSN_UID (target) % TARGET_HASH_PRIME];
	  target_hash_table[INSN_UID (target) % TARGET_HASH_PRIME] = tinfo;
	}
    }

  CLEAR_HARD_REG_SET (pending_dead_regs);

  /* If we found a basic block, get the live registers from it and update
     them with anything set or killed between its start and the insn before
     TARGET; this custom life analysis is really about registers so we need
     to use the LR problem.  Otherwise, we must assume everything is live.  */
  if (b != -1)
    {
      regset regs_live = DF_LR_IN (BASIC_BLOCK_FOR_FN (cfun, b));
      rtx_insn *start_insn, *stop_insn;
      df_ref def;

      /* Compute hard regs live at start of block.  */
      REG_SET_TO_HARD_REG_SET (current_live_regs, regs_live);
      FOR_EACH_ARTIFICIAL_DEF (def, b)
	if (DF_REF_FLAGS (def) & DF_REF_AT_TOP)
	  SET_HARD_REG_BIT (current_live_regs, DF_REF_REGNO (def));

      /* Get starting and ending insn, handling the case where each might
	 be a SEQUENCE.  */
      start_insn = (b == ENTRY_BLOCK_PTR_FOR_FN (cfun)->next_bb->index ?
		    insns : BB_HEAD (BASIC_BLOCK_FOR_FN (cfun, b)));
      stop_insn = target;

      if (NONJUMP_INSN_P (start_insn)
	  && GET_CODE (PATTERN (start_insn)) == SEQUENCE)
	start_insn = as_a <rtx_sequence *> (PATTERN (start_insn))->insn (0);

      if (NONJUMP_INSN_P (stop_insn)
	  && GET_CODE (PATTERN (stop_insn)) == SEQUENCE)
	stop_insn = next_insn (PREV_INSN (stop_insn));

      for (insn = start_insn; insn != stop_insn;
	   insn = next_insn_no_annul (insn))
	{
	  rtx link;
	  rtx_insn *real_insn = insn;
	  enum rtx_code code = GET_CODE (insn);

	  if (DEBUG_INSN_P (insn))
	    continue;

	  /* If this insn is from the target of a branch, it isn't going to
	     be used in the sequel.  If it is used in both cases, this
	     test will not be true.  */
	  if ((code == INSN || code == JUMP_INSN || code == CALL_INSN)
	      && INSN_FROM_TARGET_P (insn))
	    continue;

	  /* If this insn is a USE made by update_block, we care about the
	     underlying insn.  */
	  if (code == INSN
	      && GET_CODE (PATTERN (insn)) == USE
	      && INSN_P (XEXP (PATTERN (insn), 0)))
	    real_insn = as_a <rtx_insn *> (XEXP (PATTERN (insn), 0));

	  if (CALL_P (real_insn))
	    {
	      /* Values in call-clobbered registers survive a COND_EXEC CALL
		 if that is not executed; this matters for resoure use because
		 they may be used by a complementarily (or more strictly)
		 predicated instruction, or if the CALL is NORETURN.  */
	      if (GET_CODE (PATTERN (real_insn)) != COND_EXEC)
		{
		  HARD_REG_SET regs_invalidated_by_this_call
		    = insn_callee_abi (real_insn).full_reg_clobbers ();
		  /* CALL clobbers all call-used regs that aren't fixed except
		     sp, ap, and fp.  Do this before setting the result of the
		     call live.  */
		  current_live_regs &= ~regs_invalidated_by_this_call;
		}

	      /* A CALL_INSN sets any global register live, since it may
		 have been modified by the call.  */
	      for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
		if (global_regs[i])
		  SET_HARD_REG_BIT (current_live_regs, i);
	    }

	  /* Mark anything killed in an insn to be deadened at the next
	     label.  Ignore USE insns; the only REG_DEAD notes will be for
	     parameters.  But they might be early.  A CALL_INSN will usually
	     clobber registers used for parameters.  It isn't worth bothering
	     with the unlikely case when it won't.  */
	  if ((NONJUMP_INSN_P (real_insn)
	       && GET_CODE (PATTERN (real_insn)) != USE
	       && GET_CODE (PATTERN (real_insn)) != CLOBBER)
	      || JUMP_P (real_insn)
	      || CALL_P (real_insn))
	    {
	      for (link = REG_NOTES (real_insn); link; link = XEXP (link, 1))
		if (REG_NOTE_KIND (link) == REG_DEAD
		    && REG_P (XEXP (link, 0))
		    && REGNO (XEXP (link, 0)) < FIRST_PSEUDO_REGISTER)
		  add_to_hard_reg_set (&pending_dead_regs,
				      GET_MODE (XEXP (link, 0)),
				      REGNO (XEXP (link, 0)));

	      note_stores (real_insn, update_live_status, NULL);

	      /* If any registers were unused after this insn, kill them.
		 These notes will always be accurate.  */
	      for (link = REG_NOTES (real_insn); link; link = XEXP (link, 1))
		if (REG_NOTE_KIND (link) == REG_UNUSED
		    && REG_P (XEXP (link, 0))
		    && REGNO (XEXP (link, 0)) < FIRST_PSEUDO_REGISTER)
		  remove_from_hard_reg_set (&current_live_regs,
					   GET_MODE (XEXP (link, 0)),
					   REGNO (XEXP (link, 0)));
	    }

	  else if (LABEL_P (real_insn))
	    {
	      basic_block bb;

	      /* A label clobbers the pending dead registers since neither
		 reload nor jump will propagate a value across a label.  */
	      current_live_regs &= ~pending_dead_regs;
	      CLEAR_HARD_REG_SET (pending_dead_regs);

	      /* We must conservatively assume that all registers that used
		 to be live here still are.  The fallthrough edge may have
		 left a live register uninitialized.  */
	      bb = BLOCK_FOR_INSN (real_insn);
	      if (bb)
		{
		  HARD_REG_SET extra_live;

		  REG_SET_TO_HARD_REG_SET (extra_live, DF_LR_IN (bb));
		  current_live_regs |= extra_live;
		}
	    }

	  /* The beginning of the epilogue corresponds to the end of the
	     RTL chain when there are no epilogue insns.  Certain resources
	     are implicitly required at that point.  */
	  else if (NOTE_P (real_insn)
		   && NOTE_KIND (real_insn) == NOTE_INSN_EPILOGUE_BEG)
	    current_live_regs |= start_of_epilogue_needs.regs;
	}

      res->regs = current_live_regs;
      if (tinfo != NULL)
	{
	  tinfo->block = b;
	  tinfo->bb_tick = bb_ticks[b];
	}
    }
  else
    /* We didn't find the start of a basic block.  Assume everything
       in use.  This should happen only extremely rarely.  */
    SET_HARD_REG_SET (res->regs);

  CLEAR_RESOURCE (&set);
  CLEAR_RESOURCE (&needed);

  rtx_insn *jump_insn = find_dead_or_set_registers (target, res, &jump_target,
						    0, set, needed);

  /* If we hit an unconditional branch, we have another way of finding out
     what is live: we can see what is live at the branch target and include
     anything used but not set before the branch.  We add the live
     resources found using the test below to those found until now.  */

  if (jump_insn)
    {
      struct resources new_resources;
      rtx_insn *stop_insn = next_active_insn (jump_insn);

      if (!ANY_RETURN_P (jump_target))
	jump_target = next_active_insn (as_a<rtx_insn *> (jump_target));
      mark_target_live_regs (insns, jump_target, &new_resources);
      CLEAR_RESOURCE (&set);
      CLEAR_RESOURCE (&needed);

      /* Include JUMP_INSN in the needed registers.  */
      for (insn = target; insn != stop_insn; insn = next_active_insn (insn))
	{
	  mark_referenced_resources (insn, &needed, true);

	  scratch = needed.regs & ~set.regs;
	  new_resources.regs |= scratch;

	  mark_set_resources (insn, &set, 0, MARK_SRC_DEST_CALL);
	}

      res->regs |= new_resources.regs;
    }

  if (tinfo != NULL)
    tinfo->live_regs = res->regs;
}


// Source: resource.c
// Lines 879-1148
