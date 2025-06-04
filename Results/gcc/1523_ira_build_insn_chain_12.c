build_insn_chain (void)
{
  unsigned int i;
  class insn_chain **p = &reload_insn_chain;
  basic_block bb;
  class insn_chain *c = NULL;
  class insn_chain *next = NULL;
  auto_bitmap live_relevant_regs;
  auto_bitmap elim_regset;
  /* live_subregs is a vector used to keep accurate information about
     which hardregs are live in multiword pseudos.  live_subregs and
     live_subregs_used are indexed by pseudo number.  The live_subreg
     entry for a particular pseudo is only used if the corresponding
     element is non zero in live_subregs_used.  The sbitmap size of
     live_subreg[allocno] is number of bytes that the pseudo can
     occupy.  */
  sbitmap *live_subregs = XCNEWVEC (sbitmap, max_regno);
  auto_bitmap live_subregs_used;

  for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
    if (TEST_HARD_REG_BIT (eliminable_regset, i))
      bitmap_set_bit (elim_regset, i);
  FOR_EACH_BB_REVERSE_FN (bb, cfun)
    {
      bitmap_iterator bi;
      rtx_insn *insn;

      CLEAR_REG_SET (live_relevant_regs);
      bitmap_clear (live_subregs_used);

      EXECUTE_IF_SET_IN_BITMAP (df_get_live_out (bb), 0, i, bi)
	{
	  if (i >= FIRST_PSEUDO_REGISTER)
	    break;
	  bitmap_set_bit (live_relevant_regs, i);
	}

      EXECUTE_IF_SET_IN_BITMAP (df_get_live_out (bb),
				FIRST_PSEUDO_REGISTER, i, bi)
	{
	  if (pseudo_for_reload_consideration_p (i))
	    bitmap_set_bit (live_relevant_regs, i);
	}

      FOR_BB_INSNS_REVERSE (bb, insn)
	{
	  if (!NOTE_P (insn) && !BARRIER_P (insn))
	    {
	      struct df_insn_info *insn_info = DF_INSN_INFO_GET (insn);
	      df_ref def, use;

	      c = new_insn_chain ();
	      c->next = next;
	      next = c;
	      *p = c;
	      p = &c->prev;

	      c->insn = insn;
	      c->block = bb->index;

	      if (NONDEBUG_INSN_P (insn))
		FOR_EACH_INSN_INFO_DEF (def, insn_info)
		  {
		    unsigned int regno = DF_REF_REGNO (def);

		    /* Ignore may clobbers because these are generated
		       from calls. However, every other kind of def is
		       added to dead_or_set.  */
		    if (!DF_REF_FLAGS_IS_SET (def, DF_REF_MAY_CLOBBER))
		      {
			if (regno < FIRST_PSEUDO_REGISTER)
			  {
			    if (!fixed_regs[regno])
			      bitmap_set_bit (&c->dead_or_set, regno);
			  }
			else if (pseudo_for_reload_consideration_p (regno))
			  bitmap_set_bit (&c->dead_or_set, regno);
		      }

		    if ((regno < FIRST_PSEUDO_REGISTER
			 || reg_renumber[regno] >= 0
			 || ira_conflicts_p)
			&& (!DF_REF_FLAGS_IS_SET (def, DF_REF_CONDITIONAL)))
		      {
			rtx reg = DF_REF_REG (def);
			HOST_WIDE_INT outer_size, inner_size, start;

			/* We can usually track the liveness of individual
			   bytes within a subreg.  The only exceptions are
			   subregs wrapped in ZERO_EXTRACTs and subregs whose
			   size is not known; in those cases we need to be
			   conservative and treat the definition as a partial
			   definition of the full register rather than a full
			   definition of a specific part of the register.  */
			if (GET_CODE (reg) == SUBREG
			    && !DF_REF_FLAGS_IS_SET (def, DF_REF_ZERO_EXTRACT)
			    && get_subreg_tracking_sizes (reg, &outer_size,
							  &inner_size, &start))
			  {
			    HOST_WIDE_INT last = start + outer_size;

			    init_live_subregs
			      (bitmap_bit_p (live_relevant_regs, regno),
			       live_subregs, live_subregs_used, regno,
			       inner_size);

			    if (!DF_REF_FLAGS_IS_SET
				(def, DF_REF_STRICT_LOW_PART))
			      {
				/* Expand the range to cover entire words.
				   Bytes added here are "don't care".  */
				start
				  = start / UNITS_PER_WORD * UNITS_PER_WORD;
				last = ((last + UNITS_PER_WORD - 1)
					/ UNITS_PER_WORD * UNITS_PER_WORD);
			      }

			    /* Ignore the paradoxical bits.  */
			    if (last > SBITMAP_SIZE (live_subregs[regno]))
			      last = SBITMAP_SIZE (live_subregs[regno]);

			    while (start < last)
			      {
				bitmap_clear_bit (live_subregs[regno], start);
				start++;
			      }

			    if (bitmap_empty_p (live_subregs[regno]))
			      {
				bitmap_clear_bit (live_subregs_used, regno);
				bitmap_clear_bit (live_relevant_regs, regno);
			      }
			    else
			      /* Set live_relevant_regs here because
				 that bit has to be true to get us to
				 look at the live_subregs fields.  */
			      bitmap_set_bit (live_relevant_regs, regno);
			  }
			else
			  {
			    /* DF_REF_PARTIAL is generated for
			       subregs, STRICT_LOW_PART, and
			       ZERO_EXTRACT.  We handle the subreg
			       case above so here we have to keep from
			       modeling the def as a killing def.  */
			    if (!DF_REF_FLAGS_IS_SET (def, DF_REF_PARTIAL))
			      {
				bitmap_clear_bit (live_subregs_used, regno);
				bitmap_clear_bit (live_relevant_regs, regno);
			      }
			  }
		      }
		  }

	      bitmap_and_compl_into (live_relevant_regs, elim_regset);
	      bitmap_copy (&c->live_throughout, live_relevant_regs);

	      if (NONDEBUG_INSN_P (insn))
		FOR_EACH_INSN_INFO_USE (use, insn_info)
		  {
		    unsigned int regno = DF_REF_REGNO (use);
		    rtx reg = DF_REF_REG (use);

		    /* DF_REF_READ_WRITE on a use means that this use
		       is fabricated from a def that is a partial set
		       to a multiword reg.  Here, we only model the
		       subreg case that is not wrapped in ZERO_EXTRACT
		       precisely so we do not need to look at the
		       fabricated use.  */
		    if (DF_REF_FLAGS_IS_SET (use, DF_REF_READ_WRITE)
			&& !DF_REF_FLAGS_IS_SET (use, DF_REF_ZERO_EXTRACT)
			&& DF_REF_FLAGS_IS_SET (use, DF_REF_SUBREG))
		      continue;

		    /* Add the last use of each var to dead_or_set.  */
		    if (!bitmap_bit_p (live_relevant_regs, regno))
		      {
			if (regno < FIRST_PSEUDO_REGISTER)
			  {
			    if (!fixed_regs[regno])
			      bitmap_set_bit (&c->dead_or_set, regno);
			  }
			else if (pseudo_for_reload_consideration_p (regno))
			  bitmap_set_bit (&c->dead_or_set, regno);
		      }

		    if (regno < FIRST_PSEUDO_REGISTER
			|| pseudo_for_reload_consideration_p (regno))
		      {
			HOST_WIDE_INT outer_size, inner_size, start;
			if (GET_CODE (reg) == SUBREG
			    && !DF_REF_FLAGS_IS_SET (use,
						     DF_REF_SIGN_EXTRACT
						     | DF_REF_ZERO_EXTRACT)
			    && get_subreg_tracking_sizes (reg, &outer_size,
							  &inner_size, &start))
			  {
			    HOST_WIDE_INT last = start + outer_size;

			    init_live_subregs
			      (bitmap_bit_p (live_relevant_regs, regno),
			       live_subregs, live_subregs_used, regno,
			       inner_size);

			    /* Ignore the paradoxical bits.  */
			    if (last > SBITMAP_SIZE (live_subregs[regno]))
			      last = SBITMAP_SIZE (live_subregs[regno]);

			    while (start < last)
			      {
				bitmap_set_bit (live_subregs[regno], start);
				start++;
			      }
			  }
			else
			  /* Resetting the live_subregs_used is
			     effectively saying do not use the subregs
			     because we are reading the whole
			     pseudo.  */
			  bitmap_clear_bit (live_subregs_used, regno);
			bitmap_set_bit (live_relevant_regs, regno);
		      }
		  }
	    }
	}

      /* FIXME!! The following code is a disaster.  Reload needs to see the
	 labels and jump tables that are just hanging out in between
	 the basic blocks.  See pr33676.  */
      insn = BB_HEAD (bb);

      /* Skip over the barriers and cruft.  */
      while (insn && (BARRIER_P (insn) || NOTE_P (insn)
		      || BLOCK_FOR_INSN (insn) == bb))
	insn = PREV_INSN (insn);

      /* While we add anything except barriers and notes, the focus is
	 to get the labels and jump tables into the
	 reload_insn_chain.  */
      while (insn)
	{
	  if (!NOTE_P (insn) && !BARRIER_P (insn))
	    {
	      if (BLOCK_FOR_INSN (insn))
		break;

	      c = new_insn_chain ();
	      c->next = next;
	      next = c;
	      *p = c;
	      p = &c->prev;

	      /* The block makes no sense here, but it is what the old
		 code did.  */
	      c->block = bb->index;
	      c->insn = insn;
	      bitmap_copy (&c->live_throughout, live_relevant_regs);
	    }
	  insn = PREV_INSN (insn);
	}
    }


// Source: ira.c
// Lines 4141-4401
