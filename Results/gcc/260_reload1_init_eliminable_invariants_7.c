init_eliminable_invariants (rtx_insn *first, bool do_subregs)
{
  int i;
  rtx_insn *insn;

  grow_reg_equivs ();
  if (do_subregs)
    reg_max_ref_mode = XCNEWVEC (machine_mode, max_regno);
  else
    reg_max_ref_mode = NULL;

  num_eliminable_invariants = 0;

  first_label_num = get_first_label_num ();
  num_labels = max_label_num () - first_label_num;

  /* Allocate the tables used to store offset information at labels.  */
  offsets_known_at = XNEWVEC (char, num_labels);
  offsets_at = (poly_int64_pod (*)[NUM_ELIMINABLE_REGS])
    xmalloc (num_labels * NUM_ELIMINABLE_REGS * sizeof (poly_int64));

/* Look for REG_EQUIV notes; record what each pseudo is equivalent
   to.  If DO_SUBREGS is true, also find all paradoxical subregs and
   find largest such for each pseudo.  FIRST is the head of the insn
   list.  */

  for (insn = first; insn; insn = NEXT_INSN (insn))
    {
      rtx set = single_set (insn);

      /* We may introduce USEs that we want to remove at the end, so
	 we'll mark them with QImode.  Make sure there are no
	 previously-marked insns left by say regmove.  */
      if (INSN_P (insn) && GET_CODE (PATTERN (insn)) == USE
	  && GET_MODE (insn) != VOIDmode)
	PUT_MODE (insn, VOIDmode);

      if (do_subregs && NONDEBUG_INSN_P (insn))
	scan_paradoxical_subregs (PATTERN (insn));

      if (set != 0 && REG_P (SET_DEST (set)))
	{
	  rtx note = find_reg_note (insn, REG_EQUIV, NULL_RTX);
	  rtx x;

	  if (! note)
	    continue;

	  i = REGNO (SET_DEST (set));
	  x = XEXP (note, 0);

	  if (i <= LAST_VIRTUAL_REGISTER)
	    continue;

	  /* If flag_pic and we have constant, verify it's legitimate.  */
	  if (!CONSTANT_P (x)
	      || !flag_pic || LEGITIMATE_PIC_OPERAND_P (x))
	    {
	      /* It can happen that a REG_EQUIV note contains a MEM
		 that is not a legitimate memory operand.  As later
		 stages of reload assume that all addresses found
		 in the reg_equiv_* arrays were originally legitimate,
		 we ignore such REG_EQUIV notes.  */
	      if (memory_operand (x, VOIDmode))
		{
		  /* Always unshare the equivalence, so we can
		     substitute into this insn without touching the
		       equivalence.  */
		  reg_equiv_memory_loc (i) = copy_rtx (x);
		}
	      else if (function_invariant_p (x))
		{
		  machine_mode mode;

		  mode = GET_MODE (SET_DEST (set));
		  if (GET_CODE (x) == PLUS)
		    {
		      /* This is PLUS of frame pointer and a constant,
			 and might be shared.  Unshare it.  */
		      reg_equiv_invariant (i) = copy_rtx (x);
		      num_eliminable_invariants++;
		    }
		  else if (x == frame_pointer_rtx || x == arg_pointer_rtx)
		    {
		      reg_equiv_invariant (i) = x;
		      num_eliminable_invariants++;
		    }
		  else if (targetm.legitimate_constant_p (mode, x))
		    reg_equiv_constant (i) = x;
		  else
		    {
		      reg_equiv_memory_loc (i) = force_const_mem (mode, x);
		      if (! reg_equiv_memory_loc (i))
			reg_equiv_init (i) = NULL;
		    }
		}
	      else
		{
		  reg_equiv_init (i) = NULL;
		  continue;
		}
	    }
	  else
	    reg_equiv_init (i) = NULL;
	}
    }

  if (dump_file)
    for (i = FIRST_PSEUDO_REGISTER; i < max_regno; i++)
      if (reg_equiv_init (i))
	{
	  fprintf (dump_file, "init_insns for %u: ", i);
	  print_inline_rtx (dump_file, reg_equiv_init (i), 20);
	  fprintf (dump_file, "\n");
	}
}


// Source: reload1.c
// Lines 4008-4123
