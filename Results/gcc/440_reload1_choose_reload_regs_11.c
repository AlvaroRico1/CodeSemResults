choose_reload_regs (class insn_chain *chain)
{
  rtx_insn *insn = chain->insn;
  int i, j;
  unsigned int max_group_size = 1;
  enum reg_class group_class = NO_REGS;
  int pass, win, inheritance;

  rtx save_reload_reg_rtx[MAX_RELOADS];

  /* In order to be certain of getting the registers we need,
     we must sort the reloads into order of increasing register class.
     Then our grabbing of reload registers will parallel the process
     that provided the reload registers.

     Also note whether any of the reloads wants a consecutive group of regs.
     If so, record the maximum size of the group desired and what
     register class contains all the groups needed by this insn.  */

  for (j = 0; j < n_reloads; j++)
    {
      reload_order[j] = j;
      if (rld[j].reg_rtx != NULL_RTX)
	{
	  gcc_assert (REG_P (rld[j].reg_rtx)
		      && HARD_REGISTER_P (rld[j].reg_rtx));
	  reload_spill_index[j] = REGNO (rld[j].reg_rtx);
	}
      else
	reload_spill_index[j] = -1;

      if (rld[j].nregs > 1)
	{
	  max_group_size = MAX (rld[j].nregs, max_group_size);
	  group_class
	    = reg_class_superunion[(int) rld[j].rclass][(int) group_class];
	}

      save_reload_reg_rtx[j] = rld[j].reg_rtx;
    }

  if (n_reloads > 1)
    qsort (reload_order, n_reloads, sizeof (short), reload_reg_class_lower);

  /* If -O, try first with inheritance, then turning it off.
     If not -O, don't do inheritance.
     Using inheritance when not optimizing leads to paradoxes
     with fp on the 68k: fp numbers (not NaNs) fail to be equal to themselves
     because one side of the comparison might be inherited.  */
  win = 0;
  for (inheritance = optimize > 0; inheritance >= 0; inheritance--)
    {
      choose_reload_regs_init (chain, save_reload_reg_rtx);

      /* Process the reloads in order of preference just found.
	 Beyond this point, subregs can be found in reload_reg_rtx.

	 This used to look for an existing reloaded home for all of the
	 reloads, and only then perform any new reloads.  But that could lose
	 if the reloads were done out of reg-class order because a later
	 reload with a looser constraint might have an old home in a register
	 needed by an earlier reload with a tighter constraint.

	 To solve this, we make two passes over the reloads, in the order
	 described above.  In the first pass we try to inherit a reload
	 from a previous insn.  If there is a later reload that needs a
	 class that is a proper subset of the class being processed, we must
	 also allocate a spill register during the first pass.

	 Then make a second pass over the reloads to allocate any reloads
	 that haven't been given registers yet.  */

      for (j = 0; j < n_reloads; j++)
	{
	  int r = reload_order[j];
	  rtx search_equiv = NULL_RTX;

	  /* Ignore reloads that got marked inoperative.  */
	  if (rld[r].out == 0 && rld[r].in == 0
	      && ! rld[r].secondary_p)
	    continue;

	  /* If find_reloads chose to use reload_in or reload_out as a reload
	     register, we don't need to chose one.  Otherwise, try even if it
	     found one since we might save an insn if we find the value lying
	     around.
	     Try also when reload_in is a pseudo without a hard reg.  */
	  if (rld[r].in != 0 && rld[r].reg_rtx != 0
	      && (rtx_equal_p (rld[r].in, rld[r].reg_rtx)
		  || (rtx_equal_p (rld[r].out, rld[r].reg_rtx)
		      && !MEM_P (rld[r].in)
		      && true_regnum (rld[r].in) < FIRST_PSEUDO_REGISTER)))
	    continue;

#if 0 /* No longer needed for correct operation.
	 It might give better code, or might not; worth an experiment?  */
	  /* If this is an optional reload, we can't inherit from earlier insns
	     until we are sure that any non-optional reloads have been allocated.
	     The following code takes advantage of the fact that optional reloads
	     are at the end of reload_order.  */
	  if (rld[r].optional != 0)
	    for (i = 0; i < j; i++)
	      if ((rld[reload_order[i]].out != 0
		   || rld[reload_order[i]].in != 0
		   || rld[reload_order[i]].secondary_p)
		  && ! rld[reload_order[i]].optional
		  && rld[reload_order[i]].reg_rtx == 0)
		allocate_reload_reg (chain, reload_order[i], 0);
#endif

	  /* First see if this pseudo is already available as reloaded
	     for a previous insn.  We cannot try to inherit for reloads
	     that are smaller than the maximum number of registers needed
	     for groups unless the register we would allocate cannot be used
	     for the groups.

	     We could check here to see if this is a secondary reload for
	     an object that is already in a register of the desired class.
	     This would avoid the need for the secondary reload register.
	     But this is complex because we can't easily determine what
	     objects might want to be loaded via this reload.  So let a
	     register be allocated here.  In `emit_reload_insns' we suppress
	     one of the loads in the case described above.  */

	  if (inheritance)
	    {
	      poly_int64 byte = 0;
	      int regno = -1;
	      machine_mode mode = VOIDmode;
	      rtx subreg = NULL_RTX;

	      if (rld[r].in == 0)
		;
	      else if (REG_P (rld[r].in))
		{
		  regno = REGNO (rld[r].in);
		  mode = GET_MODE (rld[r].in);
		}
	      else if (REG_P (rld[r].in_reg))
		{
		  regno = REGNO (rld[r].in_reg);
		  mode = GET_MODE (rld[r].in_reg);
		}
	      else if (GET_CODE (rld[r].in_reg) == SUBREG
		       && REG_P (SUBREG_REG (rld[r].in_reg)))
		{
		  regno = REGNO (SUBREG_REG (rld[r].in_reg));
		  if (regno < FIRST_PSEUDO_REGISTER)
		    regno = subreg_regno (rld[r].in_reg);
		  else
		    {
		      subreg = rld[r].in_reg;
		      byte = SUBREG_BYTE (subreg);
		    }
		  mode = GET_MODE (rld[r].in_reg);
		}
#if AUTO_INC_DEC
	      else if (GET_RTX_CLASS (GET_CODE (rld[r].in_reg)) == RTX_AUTOINC
		       && REG_P (XEXP (rld[r].in_reg, 0)))
		{
		  regno = REGNO (XEXP (rld[r].in_reg, 0));
		  mode = GET_MODE (XEXP (rld[r].in_reg, 0));
		  rld[r].out = rld[r].in;
		}
#endif
#if 0
	      /* This won't work, since REGNO can be a pseudo reg number.
		 Also, it takes much more hair to keep track of all the things
		 that can invalidate an inherited reload of part of a pseudoreg.  */
	      else if (GET_CODE (rld[r].in) == SUBREG
		       && REG_P (SUBREG_REG (rld[r].in)))
		regno = subreg_regno (rld[r].in);
#endif

	      if (regno >= 0
		  && reg_last_reload_reg[regno] != 0
		  && (known_ge
		      (GET_MODE_SIZE (GET_MODE (reg_last_reload_reg[regno])),
		       GET_MODE_SIZE (mode) + byte))
		  /* Verify that the register it's in can be used in
		     mode MODE.  */
		  && (REG_CAN_CHANGE_MODE_P
		      (REGNO (reg_last_reload_reg[regno]),
		       GET_MODE (reg_last_reload_reg[regno]),
		       mode)))
		{
		  enum reg_class rclass = rld[r].rclass, last_class;
		  rtx last_reg = reg_last_reload_reg[regno];

		  i = REGNO (last_reg);
		  byte = compute_reload_subreg_offset (mode,
						       subreg,
						       GET_MODE (last_reg));
		  i += subreg_regno_offset (i, GET_MODE (last_reg), byte, mode);
		  last_class = REGNO_REG_CLASS (i);

		  if (reg_reloaded_contents[i] == regno
		      && TEST_HARD_REG_BIT (reg_reloaded_valid, i)
		      && targetm.hard_regno_mode_ok (i, rld[r].mode)
		      && (TEST_HARD_REG_BIT (reg_class_contents[(int) rclass], i)
			  /* Even if we can't use this register as a reload
			     register, we might use it for reload_override_in,
			     if copying it to the desired class is cheap
			     enough.  */
			  || ((register_move_cost (mode, last_class, rclass)
			       < memory_move_cost (mode, rclass, true))
			      && (secondary_reload_class (1, rclass, mode,
							  last_reg)
				  == NO_REGS)
			      && !(targetm.secondary_memory_needed
				   (mode, last_class, rclass))))
		      && (rld[r].nregs == max_group_size
			  || ! TEST_HARD_REG_BIT (reg_class_contents[(int) group_class],
						  i))
		      && free_for_value_p (i, rld[r].mode, rld[r].opnum,
					   rld[r].when_needed, rld[r].in,
					   const0_rtx, r, 1))
		    {
		      /* If a group is needed, verify that all the subsequent
			 registers still have their values intact.  */
		      int nr = hard_regno_nregs (i, rld[r].mode);
		      int k;

		      for (k = 1; k < nr; k++)
			if (reg_reloaded_contents[i + k] != regno
			    || ! TEST_HARD_REG_BIT (reg_reloaded_valid, i + k))
			  break;

		      if (k == nr)
			{
			  int i1;
			  int bad_for_class;

			  last_reg = (GET_MODE (last_reg) == mode
				      ? last_reg : gen_rtx_REG (mode, i));

			  bad_for_class = 0;
			  for (k = 0; k < nr; k++)
			    bad_for_class |= ! TEST_HARD_REG_BIT (reg_class_contents[(int) rld[r].rclass],
								  i+k);

			  /* We found a register that contains the
			     value we need.  If this register is the
			     same as an `earlyclobber' operand of the
			     current insn, just mark it as a place to
			     reload from since we can't use it as the
			     reload register itself.  */

			  for (i1 = 0; i1 < n_earlyclobbers; i1++)
			    if (reg_overlap_mentioned_for_reload_p
				(reg_last_reload_reg[regno],
				 reload_earlyclobbers[i1]))
			      break;

			  if (i1 != n_earlyclobbers
			      || ! (free_for_value_p (i, rld[r].mode,
						      rld[r].opnum,
						      rld[r].when_needed, rld[r].in,
						      rld[r].out, r, 1))
			      /* Don't use it if we'd clobber a pseudo reg.  */
			      || (TEST_HARD_REG_BIT (reg_used_in_insn, i)
				  && rld[r].out
				  && ! TEST_HARD_REG_BIT (reg_reloaded_dead, i))
			      /* Don't clobber the frame pointer.  */
			      || (i == HARD_FRAME_POINTER_REGNUM
				  && frame_pointer_needed
				  && rld[r].out)
			      /* Don't really use the inherited spill reg
				 if we need it wider than we've got it.  */
			      || paradoxical_subreg_p (rld[r].mode, mode)
			      || bad_for_class

			      /* If find_reloads chose reload_out as reload
				 register, stay with it - that leaves the
				 inherited register for subsequent reloads.  */
			      || (rld[r].out && rld[r].reg_rtx
				  && rtx_equal_p (rld[r].out, rld[r].reg_rtx)))
			    {
			      if (! rld[r].optional)
				{
				  reload_override_in[r] = last_reg;
				  reload_inheritance_insn[r]
				    = reg_reloaded_insn[i];
				}
			    }
			  else
			    {
			      int k;
			      /* We can use this as a reload reg.  */
			      /* Mark the register as in use for this part of
				 the insn.  */
			      mark_reload_reg_in_use (i,
						      rld[r].opnum,
						      rld[r].when_needed,
						      rld[r].mode);
			      rld[r].reg_rtx = last_reg;
			      reload_inherited[r] = 1;
			      reload_inheritance_insn[r]
				= reg_reloaded_insn[i];
			      reload_spill_index[r] = i;
			      for (k = 0; k < nr; k++)
				SET_HARD_REG_BIT (reload_reg_used_for_inherit,
						  i + k);
			    }
			}
		    }
		}
	    }

	  /* Here's another way to see if the value is already lying around.  */
	  if (inheritance
	      && rld[r].in != 0
	      && ! reload_inherited[r]
	      && rld[r].out == 0
	      && (CONSTANT_P (rld[r].in)
		  || GET_CODE (rld[r].in) == PLUS
		  || REG_P (rld[r].in)
		  || MEM_P (rld[r].in))
	      && (rld[r].nregs == max_group_size
		  || ! reg_classes_intersect_p (rld[r].rclass, group_class)))
	    search_equiv = rld[r].in;

	  if (search_equiv)
	    {
	      rtx equiv
		= find_equiv_reg (search_equiv, insn, rld[r].rclass,
				  -1, NULL, 0, rld[r].mode);
	      int regno = 0;

	      if (equiv != 0)
		{
		  if (REG_P (equiv))
		    regno = REGNO (equiv);
		  else
		    {
		      /* This must be a SUBREG of a hard register.
			 Make a new REG since this might be used in an
			 address and not all machines support SUBREGs
			 there.  */
		      gcc_assert (GET_CODE (equiv) == SUBREG);
		      regno = subreg_regno (equiv);
		      equiv = gen_rtx_REG (rld[r].mode, regno);
		      /* If we choose EQUIV as the reload register, but the
			 loop below decides to cancel the inheritance, we'll
			 end up reloading EQUIV in rld[r].mode, not the mode
			 it had originally.  That isn't safe when EQUIV isn't
			 available as a spill register since its value might
			 still be live at this point.  */
		      for (i = regno; i < regno + (int) rld[r].nregs; i++)
			if (TEST_HARD_REG_BIT (reload_reg_unavailable, i))
			  equiv = 0;
		    }
		}

	      /* If we found a spill reg, reject it unless it is free
		 and of the desired class.  */
	      if (equiv != 0)
		{
		  int regs_used = 0;
		  int bad_for_class = 0;
		  int max_regno = regno + rld[r].nregs;

		  for (i = regno; i < max_regno; i++)
		    {
		      regs_used |= TEST_HARD_REG_BIT (reload_reg_used_at_all,
						      i);
		      bad_for_class |= ! TEST_HARD_REG_BIT (reg_class_contents[(int) rld[r].rclass],
							   i);
		    }

		  if ((regs_used
		       && ! free_for_value_p (regno, rld[r].mode,
					      rld[r].opnum, rld[r].when_needed,
					      rld[r].in, rld[r].out, r, 1))
		      || bad_for_class)
		    equiv = 0;
		}

	      if (equiv != 0
		  && !targetm.hard_regno_mode_ok (regno, rld[r].mode))
		equiv = 0;

	      /* We found a register that contains the value we need.
		 If this register is the same as an `earlyclobber' operand
		 of the current insn, just mark it as a place to reload from
		 since we can't use it as the reload register itself.  */

	      if (equiv != 0)
		for (i = 0; i < n_earlyclobbers; i++)
		  if (reg_overlap_mentioned_for_reload_p (equiv,
							  reload_earlyclobbers[i]))
		    {
		      if (! rld[r].optional)
			reload_override_in[r] = equiv;
		      equiv = 0;
		      break;
		    }

	      /* If the equiv register we have found is explicitly clobbered
		 in the current insn, it depends on the reload type if we
		 can use it, use it for reload_override_in, or not at all.
		 In particular, we then can't use EQUIV for a
		 RELOAD_FOR_OUTPUT_ADDRESS reload.  */

	      if (equiv != 0)
		{
		  if (regno_clobbered_p (regno, insn, rld[r].mode, 2))
		    switch (rld[r].when_needed)
		      {
		      case RELOAD_FOR_OTHER_ADDRESS:
		      case RELOAD_FOR_INPADDR_ADDRESS:
		      case RELOAD_FOR_INPUT_ADDRESS:
		      case RELOAD_FOR_OPADDR_ADDR:
			break;
		      case RELOAD_OTHER:
		      case RELOAD_FOR_INPUT:
		      case RELOAD_FOR_OPERAND_ADDRESS:
			if (! rld[r].optional)
			  reload_override_in[r] = equiv;
			/* Fall through.  */
		      default:
			equiv = 0;
			break;
		      }
		  else if (regno_clobbered_p (regno, insn, rld[r].mode, 1))
		    switch (rld[r].when_needed)
		      {
		      case RELOAD_FOR_OTHER_ADDRESS:
		      case RELOAD_FOR_INPADDR_ADDRESS:
		      case RELOAD_FOR_INPUT_ADDRESS:
		      case RELOAD_FOR_OPADDR_ADDR:
		      case RELOAD_FOR_OPERAND_ADDRESS:
		      case RELOAD_FOR_INPUT:
			break;
		      case RELOAD_OTHER:
			if (! rld[r].optional)
			  reload_override_in[r] = equiv;
			/* Fall through.  */
		      default:
			equiv = 0;
			break;
		      }
		}

	      /* If we found an equivalent reg, say no code need be generated
		 to load it, and use it as our reload reg.  */
	      if (equiv != 0
		  && (regno != HARD_FRAME_POINTER_REGNUM
		      || !frame_pointer_needed))
		{
		  int nr = hard_regno_nregs (regno, rld[r].mode);
		  int k;
		  rld[r].reg_rtx = equiv;
		  reload_spill_index[r] = regno;
		  reload_inherited[r] = 1;

		  /* If reg_reloaded_valid is not set for this register,
		     there might be a stale spill_reg_store lying around.
		     We must clear it, since otherwise emit_reload_insns
		     might delete the store.  */
		  if (! TEST_HARD_REG_BIT (reg_reloaded_valid, regno))
		    spill_reg_store[regno] = NULL;
		  /* If any of the hard registers in EQUIV are spill
		     registers, mark them as in use for this insn.  */
		  for (k = 0; k < nr; k++)
		    {
		      i = spill_reg_order[regno + k];
		      if (i >= 0)
			{
			  mark_reload_reg_in_use (regno, rld[r].opnum,
						  rld[r].when_needed,
						  rld[r].mode);
			  SET_HARD_REG_BIT (reload_reg_used_for_inherit,
					    regno + k);
			}
		    }
		}
	    }

	  /* If we found a register to use already, or if this is an optional
	     reload, we are done.  */
	  if (rld[r].reg_rtx != 0 || rld[r].optional != 0)
	    continue;

#if 0
	  /* No longer needed for correct operation.  Might or might
	     not give better code on the average.  Want to experiment?  */

	  /* See if there is a later reload that has a class different from our
	     class that intersects our class or that requires less register
	     than our reload.  If so, we must allocate a register to this
	     reload now, since that reload might inherit a previous reload
	     and take the only available register in our class.  Don't do this
	     for optional reloads since they will force all previous reloads
	     to be allocated.  Also don't do this for reloads that have been
	     turned off.  */

	  for (i = j + 1; i < n_reloads; i++)
	    {
	      int s = reload_order[i];

	      if ((rld[s].in == 0 && rld[s].out == 0
		   && ! rld[s].secondary_p)
		  || rld[s].optional)
		continue;

	      if ((rld[s].rclass != rld[r].rclass
		   && reg_classes_intersect_p (rld[r].rclass,
					       rld[s].rclass))
		  || rld[s].nregs < rld[r].nregs)
		break;
	    }

	  if (i == n_reloads)
	    continue;

	  allocate_reload_reg (chain, r, j == n_reloads - 1);
#endif
	}

      /* Now allocate reload registers for anything non-optional that
	 didn't get one yet.  */
      for (j = 0; j < n_reloads; j++)
	{
	  int r = reload_order[j];

	  /* Ignore reloads that got marked inoperative.  */
	  if (rld[r].out == 0 && rld[r].in == 0 && ! rld[r].secondary_p)
	    continue;

	  /* Skip reloads that already have a register allocated or are
	     optional.  */
	  if (rld[r].reg_rtx != 0 || rld[r].optional)
	    continue;

	  if (! allocate_reload_reg (chain, r, j == n_reloads - 1))
	    break;
	}

      /* If that loop got all the way, we have won.  */
      if (j == n_reloads)
	{
	  win = 1;
	  break;
	}

      /* Loop around and try without any inheritance.  */
    }

  if (! win)
    {
      /* First undo everything done by the failed attempt
	 to allocate with inheritance.  */
      choose_reload_regs_init (chain, save_reload_reg_rtx);

      /* Some sanity tests to verify that the reloads found in the first
	 pass are identical to the ones we have now.  */
      gcc_assert (chain->n_reloads == n_reloads);

      for (i = 0; i < n_reloads; i++)
	{
	  if (chain->rld[i].regno < 0 || chain->rld[i].reg_rtx != 0)
	    continue;
	  gcc_assert (chain->rld[i].when_needed == rld[i].when_needed);
	  for (j = 0; j < n_spills; j++)
	    if (spill_regs[j] == chain->rld[i].regno)
	      if (! set_reload_reg (j, i))
		failed_reload (chain->insn, i);
	}
    }

  /* If we thought we could inherit a reload, because it seemed that
     nothing else wanted the same reload register earlier in the insn,
     verify that assumption, now that all reloads have been assigned.
     Likewise for reloads where reload_override_in has been set.  */

  /* If doing expensive optimizations, do one preliminary pass that doesn't
     cancel any inheritance, but removes reloads that have been needed only
     for reloads that we know can be inherited.  */
  for (pass = flag_expensive_optimizations; pass >= 0; pass--)
    {
      for (j = 0; j < n_reloads; j++)
	{
	  int r = reload_order[j];
	  rtx check_reg;
	  rtx tem;
	  if (reload_inherited[r] && rld[r].reg_rtx)
	    check_reg = rld[r].reg_rtx;
	  else if (reload_override_in[r]
		   && (REG_P (reload_override_in[r])
		       || GET_CODE (reload_override_in[r]) == SUBREG))
	    check_reg = reload_override_in[r];
	  else
	    continue;
	  if (! free_for_value_p (true_regnum (check_reg), rld[r].mode,
				  rld[r].opnum, rld[r].when_needed, rld[r].in,
				  (reload_inherited[r]
				   ? rld[r].out : const0_rtx),
				  r, 1))
	    {
	      if (pass)
		continue;
	      reload_inherited[r] = 0;
	      reload_override_in[r] = 0;
	    }
	  /* If we can inherit a RELOAD_FOR_INPUT, or can use a
	     reload_override_in, then we do not need its related
	     RELOAD_FOR_INPUT_ADDRESS / RELOAD_FOR_INPADDR_ADDRESS reloads;
	     likewise for other reload types.
	     We handle this by removing a reload when its only replacement
	     is mentioned in reload_in of the reload we are going to inherit.
	     A special case are auto_inc expressions; even if the input is
	     inherited, we still need the address for the output.  We can
	     recognize them because they have RELOAD_OUT set to RELOAD_IN.
	     If we succeeded removing some reload and we are doing a preliminary
	     pass just to remove such reloads, make another pass, since the
	     removal of one reload might allow us to inherit another one.  */
	  else if (rld[r].in
		   && rld[r].out != rld[r].in
		   && remove_address_replacements (rld[r].in))
	    {
	      if (pass)
	        pass = 2;
	    }
	  /* If we needed a memory location for the reload, we also have to
	     remove its related reloads.  */
	  else if (rld[r].in
		   && rld[r].out != rld[r].in
		   && (tem = replaced_subreg (rld[r].in), REG_P (tem))		   
		   && REGNO (tem) < FIRST_PSEUDO_REGISTER
		   && (targetm.secondary_memory_needed
		       (rld[r].inmode, REGNO_REG_CLASS (REGNO (tem)),
			rld[r].rclass))
		   && remove_address_replacements
		      (get_secondary_mem (tem, rld[r].inmode, rld[r].opnum,
					  rld[r].when_needed)))
	    {
	      if (pass)
	        pass = 2;
	    }
	}
    }

  /* Now that reload_override_in is known valid,
     actually override reload_in.  */
  for (j = 0; j < n_reloads; j++)
    if (reload_override_in[j])
      rld[j].in = reload_override_in[j];

  /* If this reload won't be done because it has been canceled or is
     optional and not inherited, clear reload_reg_rtx so other
     routines (such as subst_reloads) don't get confused.  */
  for (j = 0; j < n_reloads; j++)
    if (rld[j].reg_rtx != 0
	&& ((rld[j].optional && ! reload_inherited[j])
	    || (rld[j].in == 0 && rld[j].out == 0
		&& ! rld[j].secondary_p)))
      {
	int regno = true_regnum (rld[j].reg_rtx);

	if (spill_reg_order[regno] >= 0)
	  clear_reload_reg_in_use (regno, rld[j].opnum,
				   rld[j].when_needed, rld[j].mode);
	rld[j].reg_rtx = 0;
	reload_spill_index[j] = -1;
      }

  /* Record which pseudos and which spill regs have output reloads.  */
  for (j = 0; j < n_reloads; j++)
    {
      int r = reload_order[j];

      i = reload_spill_index[r];

      /* I is nonneg if this reload uses a register.
	 If rld[r].reg_rtx is 0, this is an optional reload
	 that we opted to ignore.  */
      if (rld[r].out_reg != 0 && REG_P (rld[r].out_reg)
	  && rld[r].reg_rtx != 0)
	{
	  int nregno = REGNO (rld[r].out_reg);
	  int nr = 1;

	  if (nregno < FIRST_PSEUDO_REGISTER)
	    nr = hard_regno_nregs (nregno, rld[r].mode);

	  while (--nr >= 0)
	    SET_REGNO_REG_SET (&reg_has_output_reload,
			       nregno + nr);

	  if (i >= 0)
	    add_to_hard_reg_set (&reg_is_output_reload, rld[r].mode, i);

	  gcc_assert (rld[r].when_needed == RELOAD_OTHER
		      || rld[r].when_needed == RELOAD_FOR_OUTPUT
		      || rld[r].when_needed == RELOAD_FOR_INSN);
	}
    }
}


// Source: reload1.c
// Lines 6302-7000
