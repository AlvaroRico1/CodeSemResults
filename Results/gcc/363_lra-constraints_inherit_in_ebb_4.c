inherit_in_ebb (rtx_insn *head, rtx_insn *tail)
{
  int i, src_regno, dst_regno, nregs;
  bool change_p, succ_p, update_reloads_num_p;
  rtx_insn *prev_insn, *last_insn;
  rtx next_usage_insns, curr_set;
  enum reg_class cl;
  struct lra_insn_reg *reg;
  basic_block last_processed_bb, curr_bb = NULL;
  HARD_REG_SET potential_reload_hard_regs, live_hard_regs;
  bitmap to_process;
  unsigned int j;
  bitmap_iterator bi;
  bool head_p, after_p;

  change_p = false;
  curr_usage_insns_check++;
  clear_invariants ();
  reloads_num = calls_num = 0;
  for (unsigned int i = 0; i < NUM_ABI_IDS; ++i)
    last_call_for_abi[i] = 0;
  CLEAR_HARD_REG_SET (full_and_partial_call_clobbers);
  bitmap_clear (&check_only_regs);
  bitmap_clear (&invalid_invariant_regs);
  last_processed_bb = NULL;
  CLEAR_HARD_REG_SET (potential_reload_hard_regs);
  live_hard_regs = eliminable_regset | lra_no_alloc_regs;
  /* We don't process new insns generated in the loop.	*/
  for (curr_insn = tail; curr_insn != PREV_INSN (head); curr_insn = prev_insn)
    {
      prev_insn = PREV_INSN (curr_insn);
      if (BLOCK_FOR_INSN (curr_insn) != NULL)
	curr_bb = BLOCK_FOR_INSN (curr_insn);
      if (last_processed_bb != curr_bb)
	{
	  /* We are at the end of BB.  Add qualified living
	     pseudos for potential splitting.  */
	  to_process = df_get_live_out (curr_bb);
	  if (last_processed_bb != NULL)
	    {
	      /* We are somewhere in the middle of EBB.	 */
	      get_live_on_other_edges (curr_bb, last_processed_bb,
				       &temp_bitmap);
	      to_process = &temp_bitmap;
	    }
	  last_processed_bb = curr_bb;
	  last_insn = get_last_insertion_point (curr_bb);
	  after_p = (! JUMP_P (last_insn)
		     && (! CALL_P (last_insn)
			 || (find_reg_note (last_insn,
					   REG_NORETURN, NULL_RTX) == NULL_RTX
			     && ! SIBLING_CALL_P (last_insn))));
	  CLEAR_HARD_REG_SET (potential_reload_hard_regs);
	  EXECUTE_IF_SET_IN_BITMAP (to_process, 0, j, bi)
	    {
	      if ((int) j >= lra_constraint_new_regno_start)
		break;
	      if (j < FIRST_PSEUDO_REGISTER || reg_renumber[j] >= 0)
		{
		  if (j < FIRST_PSEUDO_REGISTER)
		    SET_HARD_REG_BIT (live_hard_regs, j);
		  else
		    add_to_hard_reg_set (&live_hard_regs,
					 PSEUDO_REGNO_MODE (j),
					 reg_renumber[j]);
		  setup_next_usage_insn (j, last_insn, reloads_num, after_p);
		}
	    }
	}
      src_regno = dst_regno = -1;
      curr_set = single_set (curr_insn);
      if (curr_set != NULL_RTX && REG_P (SET_DEST (curr_set)))
	dst_regno = REGNO (SET_DEST (curr_set));
      if (curr_set != NULL_RTX && REG_P (SET_SRC (curr_set)))
	src_regno = REGNO (SET_SRC (curr_set));
      update_reloads_num_p = true;
      if (src_regno < lra_constraint_new_regno_start
	  && src_regno >= FIRST_PSEUDO_REGISTER
	  && reg_renumber[src_regno] < 0
	  && dst_regno >= lra_constraint_new_regno_start
	  && (cl = lra_get_allocno_class (dst_regno)) != NO_REGS)
	{
	  /* 'reload_pseudo <- original_pseudo'.  */
	  if (ira_class_hard_regs_num[cl] <= max_small_class_regs_num)
	    reloads_num++;
	  update_reloads_num_p = false;
	  succ_p = false;
	  if (usage_insns[src_regno].check == curr_usage_insns_check
	      && (next_usage_insns = usage_insns[src_regno].insns) != NULL_RTX)
	    succ_p = inherit_reload_reg (false, src_regno, cl,
					 curr_insn, next_usage_insns);
	  if (succ_p)
	    change_p = true;
	  else
	    setup_next_usage_insn (src_regno, curr_insn, reloads_num, false);
	  if (hard_reg_set_subset_p (reg_class_contents[cl], live_hard_regs))
	    potential_reload_hard_regs |= reg_class_contents[cl];
	}
      else if (src_regno < 0
	       && dst_regno >= lra_constraint_new_regno_start
	       && invariant_p (SET_SRC (curr_set))
	       && (cl = lra_get_allocno_class (dst_regno)) != NO_REGS
	       && ! bitmap_bit_p (&invalid_invariant_regs, dst_regno)
	       && ! bitmap_bit_p (&invalid_invariant_regs,
				  ORIGINAL_REGNO(regno_reg_rtx[dst_regno])))
	{
	  /* 'reload_pseudo <- invariant'.  */
	  if (ira_class_hard_regs_num[cl] <= max_small_class_regs_num)
	    reloads_num++;
	  update_reloads_num_p = false;
	  if (process_invariant_for_inheritance (SET_DEST (curr_set), SET_SRC (curr_set)))
	    change_p = true;
	  if (hard_reg_set_subset_p (reg_class_contents[cl], live_hard_regs))
	    potential_reload_hard_regs |= reg_class_contents[cl];
	}
      else if (src_regno >= lra_constraint_new_regno_start
	       && dst_regno < lra_constraint_new_regno_start
	       && dst_regno >= FIRST_PSEUDO_REGISTER
	       && reg_renumber[dst_regno] < 0
	       && (cl = lra_get_allocno_class (src_regno)) != NO_REGS
	       && usage_insns[dst_regno].check == curr_usage_insns_check
	       && (next_usage_insns
		   = usage_insns[dst_regno].insns) != NULL_RTX)
	{
	  if (ira_class_hard_regs_num[cl] <= max_small_class_regs_num)
	    reloads_num++;
	  update_reloads_num_p = false;
	  /* 'original_pseudo <- reload_pseudo'.  */
	  if (! JUMP_P (curr_insn)
	      && inherit_reload_reg (true, dst_regno, cl,
				     curr_insn, next_usage_insns))
	    change_p = true;
	  /* Invalidate.  */
	  usage_insns[dst_regno].check = 0;
	  if (hard_reg_set_subset_p (reg_class_contents[cl], live_hard_regs))
	    potential_reload_hard_regs |= reg_class_contents[cl];
	}
      else if (INSN_P (curr_insn))
	{
	  int iter;
	  int max_uid = get_max_uid ();

	  curr_id = lra_get_insn_recog_data (curr_insn);
	  curr_static_id = curr_id->insn_static_data;
	  to_inherit_num = 0;
	  /* Process insn definitions.	*/
	  for (iter = 0; iter < 2; iter++)
	    for (reg = iter == 0 ? curr_id->regs : curr_static_id->hard_regs;
		 reg != NULL;
		 reg = reg->next)
	      if (reg->type != OP_IN
		  && (dst_regno = reg->regno) < lra_constraint_new_regno_start)
		{
		  if (dst_regno >= FIRST_PSEUDO_REGISTER && reg->type == OP_OUT
		      && reg_renumber[dst_regno] < 0 && ! reg->subreg_p
		      && usage_insns[dst_regno].check == curr_usage_insns_check
		      && (next_usage_insns
			  = usage_insns[dst_regno].insns) != NULL_RTX)
		    {
		      struct lra_insn_reg *r;

		      for (r = curr_id->regs; r != NULL; r = r->next)
			if (r->type != OP_OUT && r->regno == dst_regno)
			  break;
		      /* Don't do inheritance if the pseudo is also
			 used in the insn.  */
		      if (r == NULL)
			/* We cannot do inheritance right now
			   because the current insn reg info (chain
			   regs) can change after that.  */
			add_to_inherit (dst_regno, next_usage_insns);
		    }
		  /* We cannot process one reg twice here because of
		     usage_insns invalidation.  */
		  if ((dst_regno < FIRST_PSEUDO_REGISTER
		       || reg_renumber[dst_regno] >= 0)
		      && ! reg->subreg_p && reg->type != OP_IN)
		    {
		      HARD_REG_SET s;

		      if (split_if_necessary (dst_regno, reg->biggest_mode,
					      potential_reload_hard_regs,
					      false, curr_insn, max_uid))
			change_p = true;
		      CLEAR_HARD_REG_SET (s);
		      if (dst_regno < FIRST_PSEUDO_REGISTER)
			add_to_hard_reg_set (&s, reg->biggest_mode, dst_regno);
		      else
			add_to_hard_reg_set (&s, PSEUDO_REGNO_MODE (dst_regno),
					     reg_renumber[dst_regno]);
		      live_hard_regs &= ~s;
		      potential_reload_hard_regs &= ~s;
		    }
		  /* We should invalidate potential inheritance or
		     splitting for the current insn usages to the next
		     usage insns (see code below) as the output pseudo
		     prevents this.  */
		  if ((dst_regno >= FIRST_PSEUDO_REGISTER
		       && reg_renumber[dst_regno] < 0)
		      || (reg->type == OP_OUT && ! reg->subreg_p
			  && (dst_regno < FIRST_PSEUDO_REGISTER
			      || reg_renumber[dst_regno] >= 0)))
		    {
		      /* Invalidate and mark definitions.  */
		      if (dst_regno >= FIRST_PSEUDO_REGISTER)
			usage_insns[dst_regno].check = -(int) INSN_UID (curr_insn);
		      else
			{
			  nregs = hard_regno_nregs (dst_regno,
						    reg->biggest_mode);
			  for (i = 0; i < nregs; i++)
			    usage_insns[dst_regno + i].check
			      = -(int) INSN_UID (curr_insn);
			}
		    }
		}
	  /* Process clobbered call regs.  */
	  if (curr_id->arg_hard_regs != NULL)
	    for (i = 0; (dst_regno = curr_id->arg_hard_regs[i]) >= 0; i++)
	      if (dst_regno >= FIRST_PSEUDO_REGISTER)
		usage_insns[dst_regno - FIRST_PSEUDO_REGISTER].check
		  = -(int) INSN_UID (curr_insn);
	  if (! JUMP_P (curr_insn))
	    for (i = 0; i < to_inherit_num; i++)
	      if (inherit_reload_reg (true, to_inherit[i].regno,
				      ALL_REGS, curr_insn,
				      to_inherit[i].insns))
	      change_p = true;
	  if (CALL_P (curr_insn))
	    {
	      rtx cheap, pat, dest;
	      rtx_insn *restore;
	      int regno, hard_regno;

	      calls_num++;
	      function_abi callee_abi = insn_callee_abi (curr_insn);
	      last_call_for_abi[callee_abi.id ()] = calls_num;
	      full_and_partial_call_clobbers
		|= callee_abi.full_and_partial_reg_clobbers ();
	      if ((cheap = find_reg_note (curr_insn,
					  REG_RETURNED, NULL_RTX)) != NULL_RTX
		  && ((cheap = XEXP (cheap, 0)), true)
		  && (regno = REGNO (cheap)) >= FIRST_PSEUDO_REGISTER
		  && (hard_regno = reg_renumber[regno]) >= 0
		  && usage_insns[regno].check == curr_usage_insns_check
		  /* If there are pending saves/restores, the
		     optimization is not worth.	 */
		  && usage_insns[regno].calls_num == calls_num - 1
		  && callee_abi.clobbers_reg_p (GET_MODE (cheap), hard_regno))
		{
		  /* Restore the pseudo from the call result as
		     REG_RETURNED note says that the pseudo value is
		     in the call result and the pseudo is an argument
		     of the call.  */
		  pat = PATTERN (curr_insn);
		  if (GET_CODE (pat) == PARALLEL)
		    pat = XVECEXP (pat, 0, 0);
		  dest = SET_DEST (pat);
		  /* For multiple return values dest is PARALLEL.
		     Currently we handle only single return value case.  */
		  if (REG_P (dest))
		    {
		      start_sequence ();
		      emit_move_insn (cheap, copy_rtx (dest));
		      restore = get_insns ();
		      end_sequence ();
		      lra_process_new_insns (curr_insn, NULL, restore,
					     "Inserting call parameter restore");
		      /* We don't need to save/restore of the pseudo from
			 this call.	 */
		      usage_insns[regno].calls_num = calls_num;
		      remove_from_hard_reg_set
			(&full_and_partial_call_clobbers,
			 GET_MODE (cheap), hard_regno);
		      bitmap_set_bit (&check_only_regs, regno);
		    }
		}
	    }
	  to_inherit_num = 0;
	  /* Process insn usages.  */
	  for (iter = 0; iter < 2; iter++)
	    for (reg = iter == 0 ? curr_id->regs : curr_static_id->hard_regs;
		 reg != NULL;
		 reg = reg->next)
	      if ((reg->type != OP_OUT
		   || (reg->type == OP_OUT && reg->subreg_p))
		  && (src_regno = reg->regno) < lra_constraint_new_regno_start)
		{
		  if (src_regno >= FIRST_PSEUDO_REGISTER
		      && reg_renumber[src_regno] < 0 && reg->type == OP_IN)
		    {
		      if (usage_insns[src_regno].check == curr_usage_insns_check
			  && (next_usage_insns
			      = usage_insns[src_regno].insns) != NULL_RTX
			  && NONDEBUG_INSN_P (curr_insn))
			add_to_inherit (src_regno, next_usage_insns);
		      else if (usage_insns[src_regno].check
			       != -(int) INSN_UID (curr_insn))
			/* Add usages but only if the reg is not set up
			   in the same insn.  */
			add_next_usage_insn (src_regno, curr_insn, reloads_num);
		    }
		  else if (src_regno < FIRST_PSEUDO_REGISTER
			   || reg_renumber[src_regno] >= 0)
		    {
		      bool before_p;
		      rtx_insn *use_insn = curr_insn;

		      before_p = (JUMP_P (curr_insn)
				  || (CALL_P (curr_insn) && reg->type == OP_IN));
		      if (NONDEBUG_INSN_P (curr_insn)
			  && (! JUMP_P (curr_insn) || reg->type == OP_IN)
			  && split_if_necessary (src_regno, reg->biggest_mode,
						 potential_reload_hard_regs,
						 before_p, curr_insn, max_uid))
			{
			  if (reg->subreg_p)
			    check_and_force_assignment_correctness_p = true;
			  change_p = true;
			  /* Invalidate. */
			  usage_insns[src_regno].check = 0;
			  if (before_p)
			    use_insn = PREV_INSN (curr_insn);
			}
		      if (NONDEBUG_INSN_P (curr_insn))
			{
			  if (src_regno < FIRST_PSEUDO_REGISTER)
			    add_to_hard_reg_set (&live_hard_regs,
						 reg->biggest_mode, src_regno);
			  else
			    add_to_hard_reg_set (&live_hard_regs,
						 PSEUDO_REGNO_MODE (src_regno),
						 reg_renumber[src_regno]);
			}
		      if (src_regno >= FIRST_PSEUDO_REGISTER)
			add_next_usage_insn (src_regno, use_insn, reloads_num);
		      else
			{
			  for (i = 0; i < hard_regno_nregs (src_regno, reg->biggest_mode); i++)
			    add_next_usage_insn (src_regno + i, use_insn, reloads_num);
			}
		    }


// Source: lra-constraints.c
// Lines 6282-6623
