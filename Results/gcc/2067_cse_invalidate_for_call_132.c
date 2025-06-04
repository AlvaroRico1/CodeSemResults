invalidate_for_call (rtx_insn *insn)
{
  unsigned int regno;
  unsigned hash;
  struct table_elt *p, *next;
  int in_table = 0;
  hard_reg_set_iterator hrsi;

  /* Go through all the hard registers.  For each that might be clobbered
     in call insn INSN, remove the register from quantity chains and update
     reg_tick if defined.  Also see if any of these registers is currently
     in the table.

     ??? We could be more precise for partially-clobbered registers,
     and only invalidate values that actually occupy the clobbered part
     of the registers.  It doesn't seem worth the effort though, since
     we shouldn't see this situation much before RA.  Whatever choice
     we make here has to be consistent with the table walk below,
     so any change to this test will require a change there too.  */
  HARD_REG_SET callee_clobbers
    = insn_callee_abi (insn).full_and_partial_reg_clobbers ();
  EXECUTE_IF_SET_IN_HARD_REG_SET (callee_clobbers, 0, regno, hrsi)
    {
      delete_reg_equiv (regno);
      if (REG_TICK (regno) >= 0)
	{
	  REG_TICK (regno)++;
	  SUBREG_TICKED (regno) = -1;
	}
      in_table |= (TEST_HARD_REG_BIT (hard_regs_in_table, regno) != 0);
    }

  /* In the case where we have no call-clobbered hard registers in the
     table, we are done.  Otherwise, scan the table and remove any
     entry that overlaps a call-clobbered register.  */

  if (in_table)
    for (hash = 0; hash < HASH_SIZE; hash++)
      for (p = table[hash]; p; p = next)
	{
	  next = p->next_same_hash;

	  if (!REG_P (p->exp)
	      || REGNO (p->exp) >= FIRST_PSEUDO_REGISTER)
	    continue;

	  /* This must use the same test as above rather than the
	     more accurate clobbers_reg_p.  */
	  if (overlaps_hard_reg_set_p (callee_clobbers, GET_MODE (p->exp),
				       REGNO (p->exp)))
	    remove_from_table (p, hash);
	}
}


// Source: cse.c
// Lines 2082-2134
