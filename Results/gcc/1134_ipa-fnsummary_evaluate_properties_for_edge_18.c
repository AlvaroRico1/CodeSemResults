evaluate_properties_for_edge (struct cgraph_edge *e, bool inline_p,
			      clause_t *clause_ptr,
			      clause_t *nonspec_clause_ptr,
			      vec<tree> *known_vals_ptr,
			      vec<ipa_polymorphic_call_context>
			      *known_contexts_ptr,
			      vec<ipa_agg_value_set> *known_aggs_ptr)
{
  struct cgraph_node *callee = e->callee->ultimate_alias_target ();
  class ipa_fn_summary *info = ipa_fn_summaries->get (callee);
  auto_vec<value_range, 32> known_value_ranges;
  class ipa_edge_args *args;

  if (clause_ptr)
    *clause_ptr = inline_p ? 0 : 1 << predicate::not_inlined_condition;

  if (ipa_node_params_sum
      && !e->call_stmt_cannot_inline_p
      && (info->conds || known_contexts_ptr)
      && (args = IPA_EDGE_REF (e)) != NULL)
    {
      struct cgraph_node *caller;
      class ipa_node_params *caller_parms_info, *callee_pi = NULL;
      class ipa_call_summary *es = ipa_call_summaries->get (e);
      int i, count = ipa_get_cs_argument_count (args);

      if (count)
	{
	  if (e->caller->inlined_to)
	    caller = e->caller->inlined_to;
	  else
	    caller = e->caller;
	  caller_parms_info = IPA_NODE_REF (caller);
          callee_pi = IPA_NODE_REF (callee);

	  /* Watch for thunks.  */
	  if (callee_pi)
	    /* Watch for variadic functions.  */
	    count = MIN (count, ipa_get_param_count (callee_pi));
	}

      if (callee_pi)
	for (i = 0; i < count; i++)
	  {
	    struct ipa_jump_func *jf = ipa_get_ith_jump_func (args, i);

	    if (ipa_is_param_used_by_indirect_call (callee_pi, i)
		|| ipa_is_param_used_by_ipa_predicates (callee_pi, i))
	      {
		/* Determine if we know constant value of the parameter.  */
		tree cst = ipa_value_from_jfunc (caller_parms_info, jf,
						 ipa_get_type (callee_pi, i));

		if (!cst && e->call_stmt
		    && i < (int)gimple_call_num_args (e->call_stmt))
		  {
		    cst = gimple_call_arg (e->call_stmt, i);
		    if (!is_gimple_min_invariant (cst))
		      cst = NULL;
		  }
		if (cst)
		  {
		    gcc_checking_assert (TREE_CODE (cst) != TREE_BINFO);
		    if (!known_vals_ptr->length ())
		      vec_safe_grow_cleared (known_vals_ptr, count);
		    (*known_vals_ptr)[i] = cst;
		  }
		else if (inline_p && !es->param[i].change_prob)
		  {
		    if (!known_vals_ptr->length ())
		      vec_safe_grow_cleared (known_vals_ptr, count);
		    (*known_vals_ptr)[i] = error_mark_node;
		  }

		/* If we failed to get simple constant, try value range.  */
		if ((!cst || TREE_CODE (cst) != INTEGER_CST)
		    && vrp_will_run_p (caller)
		    && ipa_is_param_used_by_ipa_predicates (callee_pi, i))
		  {
		    value_range vr 
		       = ipa_value_range_from_jfunc (caller_parms_info, e, jf,
						     ipa_get_type (callee_pi,
								   i));
		    if (!vr.undefined_p () && !vr.varying_p ())
		      {
			if (!known_value_ranges.length ())
			  known_value_ranges.safe_grow_cleared (count);
			known_value_ranges[i] = vr;
		      }
		  }

		/* Determine known aggregate values.  */
		if (fre_will_run_p (caller))
		  {
		    ipa_agg_value_set agg
			= ipa_agg_value_set_from_jfunc (caller_parms_info,
							caller, &jf->agg);
		    if (agg.items.length ())
		      {
			if (!known_aggs_ptr->length ())
			  vec_safe_grow_cleared (known_aggs_ptr, count);
			(*known_aggs_ptr)[i] = agg;
		      }
		  }
	      }

	    /* For calls used in polymorphic calls we further determine
	       polymorphic call context.  */
	    if (known_contexts_ptr
		&& ipa_is_param_used_by_polymorphic_call (callee_pi, i))
	      {
		ipa_polymorphic_call_context
		   ctx = ipa_context_from_jfunc (caller_parms_info, e, i, jf);
		if (!ctx.useless_p ())
		  {
		    if (!known_contexts_ptr->length ())
		      known_contexts_ptr->safe_grow_cleared (count);
		    (*known_contexts_ptr)[i]
		      = ipa_context_from_jfunc (caller_parms_info, e, i, jf);
		  }
	       }
	  }
	else
	  gcc_assert (!count || callee->thunk.thunk_p);
    }


// Source: ipa-fnsummary.c
// Lines 547-671
