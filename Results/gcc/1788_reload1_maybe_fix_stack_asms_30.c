maybe_fix_stack_asms (void)
{
#ifdef STACK_REGS
  const char *constraints[MAX_RECOG_OPERANDS];
  machine_mode operand_mode[MAX_RECOG_OPERANDS];
  class insn_chain *chain;

  for (chain = reload_insn_chain; chain != 0; chain = chain->next)
    {
      int i, noperands;
      HARD_REG_SET clobbered, allowed;
      rtx pat;

      if (! INSN_P (chain->insn)
	  || (noperands = asm_noperands (PATTERN (chain->insn))) < 0)
	continue;
      pat = PATTERN (chain->insn);
      if (GET_CODE (pat) != PARALLEL)
	continue;

      CLEAR_HARD_REG_SET (clobbered);
      CLEAR_HARD_REG_SET (allowed);

      /* First, make a mask of all stack regs that are clobbered.  */
      for (i = 0; i < XVECLEN (pat, 0); i++)
	{
	  rtx t = XVECEXP (pat, 0, i);
	  if (GET_CODE (t) == CLOBBER && STACK_REG_P (XEXP (t, 0)))
	    SET_HARD_REG_BIT (clobbered, REGNO (XEXP (t, 0)));
	}

      /* Get the operand values and constraints out of the insn.  */
      decode_asm_operands (pat, recog_data.operand, recog_data.operand_loc,
			   constraints, operand_mode, NULL);

      /* For every operand, see what registers are allowed.  */
      for (i = 0; i < noperands; i++)
	{
	  const char *p = constraints[i];
	  /* For every alternative, we compute the class of registers allowed
	     for reloading in CLS, and merge its contents into the reg set
	     ALLOWED.  */
	  int cls = (int) NO_REGS;

	  for (;;)
	    {
	      char c = *p;

	      if (c == '\0' || c == ',' || c == '#')
		{
		  /* End of one alternative - mark the regs in the current
		     class, and reset the class.  */
		  allowed |= reg_class_contents[cls];
		  cls = NO_REGS;
		  p++;
		  if (c == '#')
		    do {
		      c = *p++;
		    } while (c != '\0' && c != ',');
		  if (c == '\0')
		    break;
		  continue;
		}

	      switch (c)
		{
		case 'g':
		  cls = (int) reg_class_subunion[cls][(int) GENERAL_REGS];
		  break;

		default:
		  enum constraint_num cn = lookup_constraint (p);
		  if (insn_extra_address_constraint (cn))
		    cls = (int) reg_class_subunion[cls]
		      [(int) base_reg_class (VOIDmode, ADDR_SPACE_GENERIC,
					     ADDRESS, SCRATCH)];
		  else
		    cls = (int) reg_class_subunion[cls]
		      [reg_class_for_constraint (cn)];
		  break;
		}
	      p += CONSTRAINT_LEN (c, p);
	    }
	}
      /* Those of the registers which are clobbered, but allowed by the
	 constraints, must be usable as reload registers.  So clear them
	 out of the life information.  */
      allowed &= clobbered;
      for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
	if (TEST_HARD_REG_BIT (allowed, i))
	  {
	    CLEAR_REGNO_REG_SET (&chain->live_throughout, i);
	    CLEAR_REGNO_REG_SET (&chain->dead_or_set, i);
	  }
    }

#endif
}


// Source: reload1.c
// Lines 1311-1408
