reload_as_needed (int live_known)
{
  class insn_chain *chain;
#if AUTO_INC_DEC
  int i;
#endif
  rtx_note *marker;

  memset (spill_reg_rtx, 0, sizeof spill_reg_rtx);
  memset (spill_reg_store, 0, sizeof spill_reg_store);
  reg_last_reload_reg = XCNEWVEC (rtx, max_regno);
  INIT_REG_SET (&reg_has_output_reload);
  CLEAR_HARD_REG_SET (reg_reloaded_valid);

  set_initial_elim_offsets ();

  /* Generate a marker insn that we will move around.  */
  marker = emit_note (NOTE_INSN_DELETED);
  unlink_insn_chain (marker, marker);

  for (chain = reload_insn_chain; chain; chain = chain->next)
    {
      rtx_insn *prev = 0;
      rtx_insn *insn = chain->insn;
      rtx_insn *old_next = NEXT_INSN (insn);
#if AUTO_INC_DEC
      rtx_insn *old_prev = PREV_INSN (insn);
#endif

      if (will_delete_init_insn_p (insn))
	continue;

      /* If we pass a label, copy the offsets from the label information
	 into the current offsets of each elimination.  */
      if (LABEL_P (insn))
	set_offsets_for_label (insn);

      else if (INSN_P (insn))
	{
	  regset_head regs_to_forget;
	  INIT_REG_SET (&regs_to_forget);
	  note_stores (insn, forget_old_reloads_1, &regs_to_forget);

	  /* If this is a USE and CLOBBER of a MEM, ensure that any
	     references to eliminable registers have been removed.  */

	  if ((GET_CODE (PATTERN (insn)) == USE
	       || GET_CODE (PATTERN (insn)) == CLOBBER)
	      && MEM_P (XEXP (PATTERN (insn), 0)))
	    XEXP (XEXP (PATTERN (insn), 0), 0)
	      = eliminate_regs (XEXP (XEXP (PATTERN (insn), 0), 0),
				GET_MODE (XEXP (PATTERN (insn), 0)),
				NULL_RTX);

	  /* If we need to do register elimination processing, do so.
	     This might delete the insn, in which case we are done.  */
	  if ((num_eliminable || num_eliminable_invariants) && chain->need_elim)
	    {
	      eliminate_regs_in_insn (insn, 1);
	      if (NOTE_P (insn))
		{
		  update_eliminable_offsets ();
		  CLEAR_REG_SET (&regs_to_forget);
		  continue;
		}
	    }

	  /* If need_elim is nonzero but need_reload is zero, one might think
	     that we could simply set n_reloads to 0.  However, find_reloads
	     could have done some manipulation of the insn (such as swapping
	     commutative operands), and these manipulations are lost during
	     the first pass for every insn that needs register elimination.
	     So the actions of find_reloads must be redone here.  */

	  if (! chain->need_elim && ! chain->need_reload
	      && ! chain->need_operand_change)
	    n_reloads = 0;
	  /* First find the pseudo regs that must be reloaded for this insn.
	     This info is returned in the tables reload_... (see reload.h).
	     Also modify the body of INSN by substituting RELOAD
	     rtx's for those pseudo regs.  */
	  else
	    {
	      CLEAR_REG_SET (&reg_has_output_reload);
	      CLEAR_HARD_REG_SET (reg_is_output_reload);

	      find_reloads (insn, 1, spill_indirect_levels, live_known,
			    spill_reg_order);
	    }

	  if (n_reloads > 0)
	    {
	      rtx_insn *next = NEXT_INSN (insn);

	      /* ??? PREV can get deleted by reload inheritance.
		 Work around this by emitting a marker note.  */
	      prev = PREV_INSN (insn);
	      reorder_insns_nobb (marker, marker, prev);

	      /* Now compute which reload regs to reload them into.  Perhaps
		 reusing reload regs from previous insns, or else output
		 load insns to reload them.  Maybe output store insns too.
		 Record the choices of reload reg in reload_reg_rtx.  */
	      choose_reload_regs (chain);

	      /* Generate the insns to reload operands into or out of
		 their reload regs.  */
	      emit_reload_insns (chain);

	      /* Substitute the chosen reload regs from reload_reg_rtx
		 into the insn's body (or perhaps into the bodies of other
		 load and store insn that we just made for reloading
		 and that we moved the structure into).  */
	      subst_reloads (insn);

	      prev = PREV_INSN (marker);
	      unlink_insn_chain (marker, marker);

	      /* Adjust the exception region notes for loads and stores.  */
	      if (cfun->can_throw_non_call_exceptions && !CALL_P (insn))
		fixup_eh_region_note (insn, prev, next);

	      /* Adjust the location of REG_ARGS_SIZE.  */
	      rtx p = find_reg_note (insn, REG_ARGS_SIZE, NULL_RTX);
	      if (p)
		{
		  remove_note (insn, p);
		  fixup_args_size_notes (prev, PREV_INSN (next),
					 get_args_size (p));
		}

	      /* If this was an ASM, make sure that all the reload insns
		 we have generated are valid.  If not, give an error
		 and delete them.  */
	      if (asm_noperands (PATTERN (insn)) >= 0)
		for (rtx_insn *p = NEXT_INSN (prev);
		     p != next;
		     p = NEXT_INSN (p))
		  if (p != insn && INSN_P (p)
		      && GET_CODE (PATTERN (p)) != USE
		      && (recog_memoized (p) < 0
			  || (extract_insn (p),
			      !(constrain_operands (1,
				  get_enabled_alternatives (p))))))
		    {
		      error_for_asm (insn,
				     "%<asm%> operand requires "
				     "impossible reload");
		      delete_insn (p);
		    }
	    }

	  if (num_eliminable && chain->need_elim)
	    update_eliminable_offsets ();

	  /* Any previously reloaded spilled pseudo reg, stored in this insn,
	     is no longer validly lying around to save a future reload.
	     Note that this does not detect pseudos that were reloaded
	     for this insn in order to be stored in
	     (obeying register constraints).  That is correct; such reload
	     registers ARE still valid.  */
	  forget_marked_reloads (&regs_to_forget);
	  CLEAR_REG_SET (&regs_to_forget);

	  /* There may have been CLOBBER insns placed after INSN.  So scan
	     between INSN and NEXT and use them to forget old reloads.  */
	  for (rtx_insn *x = NEXT_INSN (insn); x != old_next; x = NEXT_INSN (x))
	    if (NONJUMP_INSN_P (x) && GET_CODE (PATTERN (x)) == CLOBBER)
	      note_stores (x, forget_old_reloads_1, NULL);

#if AUTO_INC_DEC
	  /* Likewise for regs altered by auto-increment in this insn.
	     REG_INC notes have been changed by reloading:
	     find_reloads_address_1 records substitutions for them,
	     which have been performed by subst_reloads above.  */
	  for (i = n_reloads - 1; i >= 0; i--)
	    {
	      rtx in_reg = rld[i].in_reg;
	      if (in_reg)
		{
		  enum rtx_code code = GET_CODE (in_reg);
		  /* PRE_INC / PRE_DEC will have the reload register ending up
		     with the same value as the stack slot, but that doesn't
		     hold true for POST_INC / POST_DEC.  Either we have to
		     convert the memory access to a true POST_INC / POST_DEC,
		     or we can't use the reload register for inheritance.  */
		  if ((code == POST_INC || code == POST_DEC)
		      && TEST_HARD_REG_BIT (reg_reloaded_valid,
					    REGNO (rld[i].reg_rtx))
		      /* Make sure it is the inc/dec pseudo, and not
			 some other (e.g. output operand) pseudo.  */
		      && ((unsigned) reg_reloaded_contents[REGNO (rld[i].reg_rtx)]
			  == REGNO (XEXP (in_reg, 0))))

		    {
		      rtx reload_reg = rld[i].reg_rtx;
		      machine_mode mode = GET_MODE (reload_reg);
		      int n = 0;
		      rtx_insn *p;

		      for (p = PREV_INSN (old_next); p != prev; p = PREV_INSN (p))
			{
			  /* We really want to ignore REG_INC notes here, so
			     use PATTERN (p) as argument to reg_set_p .  */
			  if (reg_set_p (reload_reg, PATTERN (p)))
			    break;
			  n = count_occurrences (PATTERN (p), reload_reg, 0);
			  if (! n)
			    continue;
			  if (n == 1)
			    {
			      rtx replace_reg
				= gen_rtx_fmt_e (code, mode, reload_reg);

			      validate_replace_rtx_group (reload_reg,
							  replace_reg, p);
			      n = verify_changes (0);

			      /* We must also verify that the constraints
				 are met after the replacement.  Make sure
				 extract_insn is only called for an insn
				 where the replacements were found to be
				 valid so far. */
			      if (n)
				{
				  extract_insn (p);
				  n = constrain_operands (1,
				    get_enabled_alternatives (p));
				}

			      /* If the constraints were not met, then
				 undo the replacement, else confirm it.  */
			      if (!n)
				cancel_changes (0);
			      else
				confirm_change_group ();
			    }
			  break;
			}
		      if (n == 1)
			{
			  add_reg_note (p, REG_INC, reload_reg);
			  /* Mark this as having an output reload so that the
			     REG_INC processing code below won't invalidate
			     the reload for inheritance.  */
			  SET_HARD_REG_BIT (reg_is_output_reload,
					    REGNO (reload_reg));
			  SET_REGNO_REG_SET (&reg_has_output_reload,
					     REGNO (XEXP (in_reg, 0)));
			}
		      else
			forget_old_reloads_1 (XEXP (in_reg, 0), NULL_RTX,
					      NULL);
		    }
		  else if ((code == PRE_INC || code == PRE_DEC)
			   && TEST_HARD_REG_BIT (reg_reloaded_valid,
						 REGNO (rld[i].reg_rtx))
			   /* Make sure it is the inc/dec pseudo, and not
			      some other (e.g. output operand) pseudo.  */
			   && ((unsigned) reg_reloaded_contents[REGNO (rld[i].reg_rtx)]
			       == REGNO (XEXP (in_reg, 0))))
		    {
		      SET_HARD_REG_BIT (reg_is_output_reload,
					REGNO (rld[i].reg_rtx));
		      SET_REGNO_REG_SET (&reg_has_output_reload,
					 REGNO (XEXP (in_reg, 0)));
		    }
		  else if (code == PRE_INC || code == PRE_DEC
			   || code == POST_INC || code == POST_DEC)
		    {
		      int in_regno = REGNO (XEXP (in_reg, 0));

		      if (reg_last_reload_reg[in_regno] != NULL_RTX)
			{
			  int in_hard_regno;
			  bool forget_p = true;

			  in_hard_regno = REGNO (reg_last_reload_reg[in_regno]);
			  if (TEST_HARD_REG_BIT (reg_reloaded_valid,
						 in_hard_regno))
			    {
			      for (rtx_insn *x = (old_prev ?
						  NEXT_INSN (old_prev) : insn);
				   x != old_next;
				   x = NEXT_INSN (x))
				if (x == reg_reloaded_insn[in_hard_regno])
				  {
				    forget_p = false;
				    break;
				  }
			    }
			  /* If for some reasons, we didn't set up
			     reg_last_reload_reg in this insn,
			     invalidate inheritance from previous
			     insns for the incremented/decremented
			     register.  Such registers will be not in
			     reg_has_output_reload.  Invalidate it
			     also if the corresponding element in
			     reg_reloaded_insn is also
			     invalidated.  */
			  if (forget_p)
			    forget_old_reloads_1 (XEXP (in_reg, 0),
						  NULL_RTX, NULL);
			}
		    }
		}
	    }
	  /* If a pseudo that got a hard register is auto-incremented,
	     we must purge records of copying it into pseudos without
	     hard registers.  */
	  for (rtx x = REG_NOTES (insn); x; x = XEXP (x, 1))
	    if (REG_NOTE_KIND (x) == REG_INC)
	      {
		/* See if this pseudo reg was reloaded in this insn.
		   If so, its last-reload info is still valid
		   because it is based on this insn's reload.  */
		for (i = 0; i < n_reloads; i++)
		  if (rld[i].out == XEXP (x, 0))
		    break;

		if (i == n_reloads)
		  forget_old_reloads_1 (XEXP (x, 0), NULL_RTX, NULL);
	      }
#endif
	}
      /* A reload reg's contents are unknown after a label.  */
      if (LABEL_P (insn))
	CLEAR_HARD_REG_SET (reg_reloaded_valid);

      /* Don't assume a reload reg is still good after a call insn
	 if it is a call-used reg, or if it contains a value that will
         be partially clobbered by the call.  */
      else if (CALL_P (insn))
	{
	  reg_reloaded_valid
	    &= ~insn_callee_abi (insn).full_and_partial_reg_clobbers ();

	  /* If this is a call to a setjmp-type function, we must not
	     reuse any reload reg contents across the call; that will
	     just be clobbered by other uses of the register in later
	     code, before the longjmp.  */
	  if (find_reg_note (insn, REG_SETJMP, NULL_RTX))
	    CLEAR_HARD_REG_SET (reg_reloaded_valid);
	}
    }

  /* Clean up.  */
  free (reg_last_reload_reg);
  CLEAR_REG_SET (&reg_has_output_reload);
}


// Source: reload1.c
// Lines 4440-4789
