param_change_prob (ipa_func_body_info *fbi, gimple *stmt, int i)
{
  tree op = gimple_call_arg (stmt, i);
  basic_block bb = gimple_bb (stmt);

  if (TREE_CODE (op) == WITH_SIZE_EXPR)
    op = TREE_OPERAND (op, 0);

  tree base = get_base_address (op);

  /* Global invariants never change.  */
  if (is_gimple_min_invariant (base))
    return 0;

  /* We would have to do non-trivial analysis to really work out what
     is the probability of value to change (i.e. when init statement
     is in a sibling loop of the call). 

     We do an conservative estimate: when call is executed N times more often
     than the statement defining value, we take the frequency 1/N.  */
  if (TREE_CODE (base) == SSA_NAME)
    {
      profile_count init_count;

      if (!bb->count.nonzero_p ())
	return REG_BR_PROB_BASE;

      if (SSA_NAME_IS_DEFAULT_DEF (base))
	init_count = ENTRY_BLOCK_PTR_FOR_FN (cfun)->count;
      else
	init_count = get_minimal_bb
		      (gimple_bb (SSA_NAME_DEF_STMT (base)),
		       gimple_bb (stmt))->count;

      if (init_count < bb->count)
        return MAX ((init_count.to_sreal_scale (bb->count)
		     * REG_BR_PROB_BASE).to_int (), 1);
      return REG_BR_PROB_BASE;
    }
  else
    {
      ao_ref refd;
      profile_count max = ENTRY_BLOCK_PTR_FOR_FN (cfun)->count;
      struct record_modified_bb_info info;
      tree init = ctor_for_folding (base);

      if (init != error_mark_node)
	return 0;
      if (!bb->count.nonzero_p ())
	return REG_BR_PROB_BASE;
      if (dump_file)
	{
	  fprintf (dump_file, "     Analyzing param change probability of ");
          print_generic_expr (dump_file, op, TDF_SLIM);
	  fprintf (dump_file, "\n");
	}
      ao_ref_init (&refd, op);
      info.op = op;
      info.stmt = stmt;
      info.bb_set = BITMAP_ALLOC (NULL);
      int walked
	= walk_aliased_vdefs (&refd, gimple_vuse (stmt), record_modified, &info,
			      NULL, NULL, fbi->aa_walk_budget);
      if (walked < 0 || bitmap_bit_p (info.bb_set, bb->index))
	{
	  if (dump_file)
	    {
	      if (walked < 0)
		fprintf (dump_file, "     Ran out of AA walking budget.\n");
	      else
		fprintf (dump_file, "     Set in same BB as used.\n");
	    }
	  BITMAP_FREE (info.bb_set);
	  return REG_BR_PROB_BASE;
	}

      bitmap_iterator bi;
      unsigned index;
      /* Lookup the most frequent update of the value and believe that
	 it dominates all the other; precise analysis here is difficult.  */
      EXECUTE_IF_SET_IN_BITMAP (info.bb_set, 0, index, bi)
	max = max.max (BASIC_BLOCK_FOR_FN (cfun, index)->count);
      if (dump_file)
	{
          fprintf (dump_file, "     Set with count ");	
	  max.dump (dump_file);
          fprintf (dump_file, " and used with count ");	
	  bb->count.dump (dump_file);
          fprintf (dump_file, " freq %f\n",
		   max.to_sreal_scale (bb->count).to_double ());	
	}

      BITMAP_FREE (info.bb_set);
      if (max < bb->count)
        return MAX ((max.to_sreal_scale (bb->count)
		     * REG_BR_PROB_BASE).to_int (), 1);
      return REG_BR_PROB_BASE;
    }


// Source: ipa-fnsummary.c
// Lines 2098-2195
