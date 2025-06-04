emit_reload_insns (class insn_chain *chain)
{
  rtx_insn *insn = chain->insn;

  int j;

  CLEAR_HARD_REG_SET (reg_reloaded_died);

  for (j = 0; j < reload_n_operands; j++)
    input_reload_insns[j] = input_address_reload_insns[j]
      = inpaddr_address_reload_insns[j]
      = output_reload_insns[j] = output_address_reload_insns[j]
      = outaddr_address_reload_insns[j]
      = other_output_reload_insns[j] = 0;
  other_input_address_reload_insns = 0;
  other_input_reload_insns = 0;
  operand_reload_insns = 0;
  other_operand_reload_insns = 0;

  /* Dump reloads into the dump file.  */
  if (dump_file)
    {
      fprintf (dump_file, "\nReloads for insn # %d\n", INSN_UID (insn));
      debug_reload_to_stream (dump_file);
    }

  for (j = 0; j < n_reloads; j++)
    if (rld[j].reg_rtx && HARD_REGISTER_P (rld[j].reg_rtx))
      {
	unsigned int i;

	for (i = REGNO (rld[j].reg_rtx); i < END_REGNO (rld[j].reg_rtx); i++)
	  new_spill_reg_store[i] = 0;
      }

  /* Now output the instructions to copy the data into and out of the
     reload registers.  Do these in the order that the reloads were reported,
     since reloads of base and index registers precede reloads of operands
     and the operands may need the base and index registers reloaded.  */

  for (j = 0; j < n_reloads; j++)
    {
      do_input_reload (chain, rld + j, j);
      do_output_reload (chain, rld + j, j);
    }

  /* Now write all the insns we made for reloads in the order expected by
     the allocation functions.  Prior to the insn being reloaded, we write
     the following reloads:

     RELOAD_FOR_OTHER_ADDRESS reloads for input addresses.

     RELOAD_OTHER reloads.

     For each operand, any RELOAD_FOR_INPADDR_ADDRESS reloads followed
     by any RELOAD_FOR_INPUT_ADDRESS reloads followed by the
     RELOAD_FOR_INPUT reload for the operand.

     RELOAD_FOR_OPADDR_ADDRS reloads.

     RELOAD_FOR_OPERAND_ADDRESS reloads.

     After the insn being reloaded, we write the following:

     For each operand, any RELOAD_FOR_OUTADDR_ADDRESS reloads followed
     by any RELOAD_FOR_OUTPUT_ADDRESS reload followed by the
     RELOAD_FOR_OUTPUT reload, followed by any RELOAD_OTHER output
     reloads for the operand.  The RELOAD_OTHER output reloads are
     output in descending order by reload number.  */

  emit_insn_before (other_input_address_reload_insns, insn);
  emit_insn_before (other_input_reload_insns, insn);

  for (j = 0; j < reload_n_operands; j++)
    {
      emit_insn_before (inpaddr_address_reload_insns[j], insn);
      emit_insn_before (input_address_reload_insns[j], insn);
      emit_insn_before (input_reload_insns[j], insn);
    }

  emit_insn_before (other_operand_reload_insns, insn);
  emit_insn_before (operand_reload_insns, insn);

  for (j = 0; j < reload_n_operands; j++)
    {
      rtx_insn *x = emit_insn_after (outaddr_address_reload_insns[j], insn);
      x = emit_insn_after (output_address_reload_insns[j], x);
      x = emit_insn_after (output_reload_insns[j], x);
      emit_insn_after (other_output_reload_insns[j], x);
    }

  /* For all the spill regs newly reloaded in this instruction,
     record what they were reloaded from, so subsequent instructions
     can inherit the reloads.

     Update spill_reg_store for the reloads of this insn.
     Copy the elements that were updated in the loop above.  */

  for (j = 0; j < n_reloads; j++)
    {
      int r = reload_order[j];
      int i = reload_spill_index[r];

      /* If this is a non-inherited input reload from a pseudo, we must
	 clear any memory of a previous store to the same pseudo.  Only do
	 something if there will not be an output reload for the pseudo
	 being reloaded.  */
      if (rld[r].in_reg != 0
	  && ! (reload_inherited[r] || reload_override_in[r]))
	{
	  rtx reg = rld[r].in_reg;

	  if (GET_CODE (reg) == SUBREG)
	    reg = SUBREG_REG (reg);

	  if (REG_P (reg)
	      && REGNO (reg) >= FIRST_PSEUDO_REGISTER
	      && !REGNO_REG_SET_P (&reg_has_output_reload, REGNO (reg)))
	    {
	      int nregno = REGNO (reg);

	      if (reg_last_reload_reg[nregno])
		{
		  int last_regno = REGNO (reg_last_reload_reg[nregno]);

		  if (reg_reloaded_contents[last_regno] == nregno)
		    spill_reg_store[last_regno] = 0;
		}
	    }
	}

      /* I is nonneg if this reload used a register.
	 If rld[r].reg_rtx is 0, this is an optional reload
	 that we opted to ignore.  */

      if (i >= 0 && rld[r].reg_rtx != 0)
	{
	  int nr = hard_regno_nregs (i, GET_MODE (rld[r].reg_rtx));
	  int k;

	  /* For a multi register reload, we need to check if all or part
	     of the value lives to the end.  */
	  for (k = 0; k < nr; k++)
	    if (reload_reg_reaches_end_p (i + k, r))
	      CLEAR_HARD_REG_BIT (reg_reloaded_valid, i + k);

	  /* Maybe the spill reg contains a copy of reload_out.  */
	  if (rld[r].out != 0
	      && (REG_P (rld[r].out)
		  || (rld[r].out_reg
		      ? REG_P (rld[r].out_reg)
		      /* The reload value is an auto-modification of
			 some kind.  For PRE_INC, POST_INC, PRE_DEC
			 and POST_DEC, we record an equivalence
			 between the reload register and the operand
			 on the optimistic assumption that we can make
			 the equivalence hold.  reload_as_needed must
			 then either make it hold or invalidate the
			 equivalence.

			 PRE_MODIFY and POST_MODIFY addresses are reloaded
			 somewhat differently, and allowing them here leads
			 to problems.  */
		      : (GET_CODE (rld[r].out) != POST_MODIFY
			 && GET_CODE (rld[r].out) != PRE_MODIFY))))
	    {
	      rtx reg;

	      reg = reload_reg_rtx_for_output[r];
	      if (reload_reg_rtx_reaches_end_p (reg, r))
		{
		  machine_mode mode = GET_MODE (reg);
		  int regno = REGNO (reg);
		  int nregs = REG_NREGS (reg);
		  rtx out = (REG_P (rld[r].out)
			     ? rld[r].out
			     : rld[r].out_reg
			     ? rld[r].out_reg
/* AUTO_INC */		     : XEXP (rld[r].in_reg, 0));
		  int out_regno = REGNO (out);
		  int out_nregs = (!HARD_REGISTER_NUM_P (out_regno) ? 1
				   : hard_regno_nregs (out_regno, mode));
		  bool piecemeal;

		  spill_reg_store[regno] = new_spill_reg_store[regno];
		  spill_reg_stored_to[regno] = out;
		  reg_last_reload_reg[out_regno] = reg;

		  piecemeal = (HARD_REGISTER_NUM_P (out_regno)
			       && nregs == out_nregs
			       && inherit_piecemeal_p (out_regno, regno, mode));

		  /* If OUT_REGNO is a hard register, it may occupy more than
		     one register.  If it does, say what is in the
		     rest of the registers assuming that both registers
		     agree on how many words the object takes.  If not,
		     invalidate the subsequent registers.  */

		  if (HARD_REGISTER_NUM_P (out_regno))
		    for (k = 1; k < out_nregs; k++)
		      reg_last_reload_reg[out_regno + k]
			= (piecemeal ? regno_reg_rtx[regno + k] : 0);

		  /* Now do the inverse operation.  */
		  for (k = 0; k < nregs; k++)
		    {
		      CLEAR_HARD_REG_BIT (reg_reloaded_dead, regno + k);
		      reg_reloaded_contents[regno + k]
			= (!HARD_REGISTER_NUM_P (out_regno) || !piecemeal
			   ? out_regno
			   : out_regno + k);
		      reg_reloaded_insn[regno + k] = insn;
		      SET_HARD_REG_BIT (reg_reloaded_valid, regno + k);
		    }
		}
	    }
	  /* Maybe the spill reg contains a copy of reload_in.  Only do
	     something if there will not be an output reload for
	     the register being reloaded.  */
	  else if (rld[r].out_reg == 0
		   && rld[r].in != 0
		   && ((REG_P (rld[r].in)
			&& !HARD_REGISTER_P (rld[r].in)
			&& !REGNO_REG_SET_P (&reg_has_output_reload,
					     REGNO (rld[r].in)))
		       || (REG_P (rld[r].in_reg)
			   && !REGNO_REG_SET_P (&reg_has_output_reload,
						REGNO (rld[r].in_reg))))
		   && !reg_set_p (reload_reg_rtx_for_input[r], PATTERN (insn)))
	    {
	      rtx reg;

	      reg = reload_reg_rtx_for_input[r];
	      if (reload_reg_rtx_reaches_end_p (reg, r))
		{
		  machine_mode mode;
		  int regno;
		  int nregs;
		  int in_regno;
		  int in_nregs;
		  rtx in;
		  bool piecemeal;

		  mode = GET_MODE (reg);
		  regno = REGNO (reg);
		  nregs = REG_NREGS (reg);
		  if (REG_P (rld[r].in)
		      && REGNO (rld[r].in) >= FIRST_PSEUDO_REGISTER)
		    in = rld[r].in;
		  else if (REG_P (rld[r].in_reg))
		    in = rld[r].in_reg;
		  else
		    in = XEXP (rld[r].in_reg, 0);
		  in_regno = REGNO (in);

		  in_nregs = (!HARD_REGISTER_NUM_P (in_regno) ? 1
			      : hard_regno_nregs (in_regno, mode));

		  reg_last_reload_reg[in_regno] = reg;

		  piecemeal = (HARD_REGISTER_NUM_P (in_regno)
			       && nregs == in_nregs
			       && inherit_piecemeal_p (regno, in_regno, mode));

		  if (HARD_REGISTER_NUM_P (in_regno))
		    for (k = 1; k < in_nregs; k++)
		      reg_last_reload_reg[in_regno + k]
			= (piecemeal ? regno_reg_rtx[regno + k] : 0);

		  /* Unless we inherited this reload, show we haven't
		     recently done a store.
		     Previous stores of inherited auto_inc expressions
		     also have to be discarded.  */
		  if (! reload_inherited[r]
		      || (rld[r].out && ! rld[r].out_reg))
		    spill_reg_store[regno] = 0;

		  for (k = 0; k < nregs; k++)
		    {
		      CLEAR_HARD_REG_BIT (reg_reloaded_dead, regno + k);
		      reg_reloaded_contents[regno + k]
			= (!HARD_REGISTER_NUM_P (in_regno) || !piecemeal
			   ? in_regno
			   : in_regno + k);
		      reg_reloaded_insn[regno + k] = insn;
		      SET_HARD_REG_BIT (reg_reloaded_valid, regno + k);
		    }
		}
	    }
	}

      /* The following if-statement was #if 0'd in 1.34 (or before...).
	 It's reenabled in 1.35 because supposedly nothing else
	 deals with this problem.  */

      /* If a register gets output-reloaded from a non-spill register,
	 that invalidates any previous reloaded copy of it.
	 But forget_old_reloads_1 won't get to see it, because
	 it thinks only about the original insn.  So invalidate it here.
	 Also do the same thing for RELOAD_OTHER constraints where the
	 output is discarded.  */
      if (i < 0
	  && ((rld[r].out != 0
	       && (REG_P (rld[r].out)
		   || (MEM_P (rld[r].out)
		       && REG_P (rld[r].out_reg))))
	      || (rld[r].out == 0 && rld[r].out_reg
		  && REG_P (rld[r].out_reg))))
	{
	  rtx out = ((rld[r].out && REG_P (rld[r].out))
		     ? rld[r].out : rld[r].out_reg);
	  int out_regno = REGNO (out);
	  machine_mode mode = GET_MODE (out);

	  /* REG_RTX is now set or clobbered by the main instruction.
	     As the comment above explains, forget_old_reloads_1 only
	     sees the original instruction, and there is no guarantee
	     that the original instruction also clobbered REG_RTX.
	     For example, if find_reloads sees that the input side of
	     a matched operand pair dies in this instruction, it may
	     use the input register as the reload register.

	     Calling forget_old_reloads_1 is a waste of effort if
	     REG_RTX is also the output register.

	     If we know that REG_RTX holds the value of a pseudo
	     register, the code after the call will record that fact.  */
	  if (rld[r].reg_rtx && rld[r].reg_rtx != out)
	    forget_old_reloads_1 (rld[r].reg_rtx, NULL_RTX, NULL);

	  if (!HARD_REGISTER_NUM_P (out_regno))
	    {
	      rtx src_reg;
	      rtx_insn *store_insn = NULL;

	      reg_last_reload_reg[out_regno] = 0;

	      /* If we can find a hard register that is stored, record
		 the storing insn so that we may delete this insn with
		 delete_output_reload.  */
	      src_reg = reload_reg_rtx_for_output[r];

	      if (src_reg)
		{
		  if (reload_reg_rtx_reaches_end_p (src_reg, r))
		    store_insn = new_spill_reg_store[REGNO (src_reg)];
		  else
		    src_reg = NULL_RTX;
		}
	      else
		{
		  /* If this is an optional reload, try to find the
		     source reg from an input reload.  */
		  rtx set = single_set (insn);
		  if (set && SET_DEST (set) == rld[r].out)
		    {
		      int k;

		      src_reg = SET_SRC (set);
		      store_insn = insn;
		      for (k = 0; k < n_reloads; k++)
			{
			  if (rld[k].in == src_reg)
			    {
			      src_reg = reload_reg_rtx_for_input[k];
			      break;
			    }
			}
		    }
		}
	      if (src_reg && REG_P (src_reg)
		  && REGNO (src_reg) < FIRST_PSEUDO_REGISTER)
		{
		  int src_regno, src_nregs, k;
		  rtx note;

		  gcc_assert (GET_MODE (src_reg) == mode);
		  src_regno = REGNO (src_reg);
		  src_nregs = hard_regno_nregs (src_regno, mode);
		  /* The place where to find a death note varies with
		     PRESERVE_DEATH_INFO_REGNO_P .  The condition is not
		     necessarily checked exactly in the code that moves
		     notes, so just check both locations.  */
		  note = find_regno_note (insn, REG_DEAD, src_regno);
		  if (! note && store_insn)
		    note = find_regno_note (store_insn, REG_DEAD, src_regno);
		  for (k = 0; k < src_nregs; k++)
		    {
		      spill_reg_store[src_regno + k] = store_insn;
		      spill_reg_stored_to[src_regno + k] = out;
		      reg_reloaded_contents[src_regno + k] = out_regno;
		      reg_reloaded_insn[src_regno + k] = store_insn;
		      CLEAR_HARD_REG_BIT (reg_reloaded_dead, src_regno + k);
		      SET_HARD_REG_BIT (reg_reloaded_valid, src_regno + k);
		      SET_HARD_REG_BIT (reg_is_output_reload, src_regno + k);
		      if (note)
			SET_HARD_REG_BIT (reg_reloaded_died, src_regno);
		      else
			CLEAR_HARD_REG_BIT (reg_reloaded_died, src_regno);
		    }
		  reg_last_reload_reg[out_regno] = src_reg;
		  /* We have to set reg_has_output_reload here, or else
		     forget_old_reloads_1 will clear reg_last_reload_reg
		     right away.  */
		  SET_REGNO_REG_SET (&reg_has_output_reload,
				     out_regno);
		}
	    }


// Source: reload1.c
// Lines 7964-8371
