finish_spills (int global)
{
  class insn_chain *chain;
  int something_changed = 0;
  unsigned i;
  reg_set_iterator rsi;

  /* Build the spill_regs array for the function.  */
  /* If there are some registers still to eliminate and one of the spill regs
     wasn't ever used before, additional stack space may have to be
     allocated to store this register.  Thus, we may have changed the offset
     between the stack and frame pointers, so mark that something has changed.

     One might think that we need only set VAL to 1 if this is a call-used
     register.  However, the set of registers that must be saved by the
     prologue is not identical to the call-used set.  For example, the
     register used by the call insn for the return PC is a call-used register,
     but must be saved by the prologue.  */

  n_spills = 0;
  for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
    if (TEST_HARD_REG_BIT (used_spill_regs, i))
      {
	spill_reg_order[i] = n_spills;
	spill_regs[n_spills++] = i;
	if (num_eliminable && ! df_regs_ever_live_p (i))
	  something_changed = 1;
	df_set_regs_ever_live (i, true);
      }
    else
      spill_reg_order[i] = -1;

  EXECUTE_IF_SET_IN_REG_SET (&spilled_pseudos, FIRST_PSEUDO_REGISTER, i, rsi)
    if (reg_renumber[i] >= 0)
      {
	SET_HARD_REG_BIT (pseudo_previous_regs[i], reg_renumber[i]);
	/* Mark it as no longer having a hard register home.  */
	reg_renumber[i] = -1;
	if (ira_conflicts_p)
	  /* Inform IRA about the change.  */
	  ira_mark_allocation_change (i);
	/* We will need to scan everything again.  */
	something_changed = 1;
      }

  /* Retry global register allocation if possible.  */
  if (global && ira_conflicts_p)
    {
      unsigned int n;

      memset (pseudo_forbidden_regs, 0, max_regno * sizeof (HARD_REG_SET));
      /* For every insn that needs reloads, set the registers used as spill
	 regs in pseudo_forbidden_regs for every pseudo live across the
	 insn.  */
      for (chain = insns_need_reload; chain; chain = chain->next_need_reload)
	{
	  EXECUTE_IF_SET_IN_REG_SET
	    (&chain->live_throughout, FIRST_PSEUDO_REGISTER, i, rsi)
	    {
	      pseudo_forbidden_regs[i] |= chain->used_spill_regs;
	    }
	  EXECUTE_IF_SET_IN_REG_SET
	    (&chain->dead_or_set, FIRST_PSEUDO_REGISTER, i, rsi)
	    {
	      pseudo_forbidden_regs[i] |= chain->used_spill_regs;
	    }
	}

      /* Retry allocating the pseudos spilled in IRA and the
	 reload.  For each reg, merge the various reg sets that
	 indicate which hard regs can't be used, and call
	 ira_reassign_pseudos.  */
      for (n = 0, i = FIRST_PSEUDO_REGISTER; i < (unsigned) max_regno; i++)
	if (reg_old_renumber[i] != reg_renumber[i])
	  {
	    if (reg_renumber[i] < 0)
	      temp_pseudo_reg_arr[n++] = i;
	    else
	      CLEAR_REGNO_REG_SET (&spilled_pseudos, i);
	  }
      if (ira_reassign_pseudos (temp_pseudo_reg_arr, n,
				bad_spill_regs_global,
				pseudo_forbidden_regs, pseudo_previous_regs,
				&spilled_pseudos))
	something_changed = 1;
    }
  /* Fix up the register information in the insn chain.
     This involves deleting those of the spilled pseudos which did not get
     a new hard register home from the live_{before,after} sets.  */
  for (chain = reload_insn_chain; chain; chain = chain->next)
    {
      HARD_REG_SET used_by_pseudos;
      HARD_REG_SET used_by_pseudos2;

      if (! ira_conflicts_p)
	{
	  /* Don't do it for IRA because IRA and the reload still can
	     assign hard registers to the spilled pseudos on next
	     reload iterations.  */
	  AND_COMPL_REG_SET (&chain->live_throughout, &spilled_pseudos);
	  AND_COMPL_REG_SET (&chain->dead_or_set, &spilled_pseudos);
	}
      /* Mark any unallocated hard regs as available for spills.  That
	 makes inheritance work somewhat better.  */
      if (chain->need_reload)
	{
	  REG_SET_TO_HARD_REG_SET (used_by_pseudos, &chain->live_throughout);
	  REG_SET_TO_HARD_REG_SET (used_by_pseudos2, &chain->dead_or_set);
	  used_by_pseudos |= used_by_pseudos2;

	  compute_use_by_pseudos (&used_by_pseudos, &chain->live_throughout);
	  compute_use_by_pseudos (&used_by_pseudos, &chain->dead_or_set);
	  /* Value of chain->used_spill_regs from previous iteration
	     may be not included in the value calculated here because
	     of possible removing caller-saves insns (see function
	     delete_caller_save_insns.  */
	  chain->used_spill_regs = ~used_by_pseudos & used_spill_regs;
	}
    }

  CLEAR_REG_SET (&changed_allocation_pseudos);
  /* Let alter_reg modify the reg rtx's for the modified pseudos.  */
  for (i = FIRST_PSEUDO_REGISTER; i < (unsigned)max_regno; i++)
    {
      int regno = reg_renumber[i];
      if (reg_old_renumber[i] == regno)
	continue;

      SET_REGNO_REG_SET (&changed_allocation_pseudos, i);

      alter_reg (i, reg_old_renumber[i], false);
      reg_old_renumber[i] = regno;
      if (dump_file)
	{
	  if (regno == -1)
	    fprintf (dump_file, " Register %d now on stack.\n\n", i);
	  else
	    fprintf (dump_file, " Register %d now in %d.\n\n",
		     i, reg_renumber[i]);
	}
    }

  return something_changed;
}


// Source: reload1.c
// Lines 4184-4327
