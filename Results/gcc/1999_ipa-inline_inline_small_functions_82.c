inline_small_functions (void)
{
  struct cgraph_node *node;
  struct cgraph_edge *edge;
  edge_heap_t edge_heap (sreal::min ());
  auto_bitmap updated_nodes;
  int min_size;
  auto_vec<cgraph_edge *> new_indirect_edges;
  int initial_size = 0;
  struct cgraph_node **order = XCNEWVEC (cgraph_node *, symtab->cgraph_count);
  struct cgraph_edge_hook_list *edge_removal_hook_holder;
  new_indirect_edges.create (8);

  edge_removal_hook_holder
    = symtab->add_edge_removal_hook (&heap_edge_removal_hook, &edge_heap);

  /* Compute overall unit size and other global parameters used by badness
     metrics.  */

  max_count = profile_count::uninitialized ();
  ipa_reduced_postorder (order, true, ignore_edge_p);
  free (order);

  FOR_EACH_DEFINED_FUNCTION (node)
    if (!node->inlined_to)
      {
	if (!node->alias && node->analyzed
	    && (node->has_gimple_body_p () || node->thunk.thunk_p)
	    && opt_for_fn (node->decl, optimize))
	  {
	    class ipa_fn_summary *info = ipa_fn_summaries->get (node);
	    struct ipa_dfs_info *dfs = (struct ipa_dfs_info *) node->aux;

	    /* Do not account external functions, they will be optimized out
	       if not inlined.  Also only count the non-cold portion of program.  */
	    if (inline_account_function_p (node))
	      initial_size += ipa_size_summaries->get (node)->size;
	    info->growth = estimate_growth (node);

	    int num_calls = 0;
	    node->call_for_symbol_and_aliases (sum_callers, &num_calls,
					       true);
	    if (num_calls == 1)
	      info->single_caller = true;
	    if (dfs && dfs->next_cycle)
	      {
		struct cgraph_node *n2;
		int id = dfs->scc_no + 1;
		for (n2 = node; n2;
		     n2 = ((struct ipa_dfs_info *) n2->aux)->next_cycle)
		  if (opt_for_fn (n2->decl, optimize))
		    {
		      ipa_fn_summary *info2 = ipa_fn_summaries->get
			 (n2->inlined_to ? n2->inlined_to : n2);
		      if (info2->scc_no)
			break;
		      info2->scc_no = id;
		    }
	      }
	  }

	for (edge = node->callers; edge; edge = edge->next_caller)
	  max_count = max_count.max (edge->count.ipa ());
      }
  ipa_free_postorder_info ();
  initialize_growth_caches ();

  if (dump_file)
    fprintf (dump_file,
	     "\nDeciding on inlining of small functions.  Starting with size %i.\n",
	     initial_size);

  overall_size = initial_size;
  min_size = overall_size;

  /* Populate the heap with all edges we might inline.  */

  FOR_EACH_DEFINED_FUNCTION (node)
    {
      bool update = false;
      struct cgraph_edge *next = NULL;
      bool has_speculative = false;

      if (!opt_for_fn (node->decl, optimize))
	continue;

      if (dump_file)
	fprintf (dump_file, "Enqueueing calls in %s.\n", node->dump_name ());

      for (edge = node->callees; edge; edge = edge->next_callee)
	{
	  if (edge->inline_failed
	      && !edge->aux
	      && can_inline_edge_p (edge, true)
	      && want_inline_small_function_p (edge, true)
	      && can_inline_edge_by_limits_p (edge, true)
	      && edge->inline_failed)
	    {
	      gcc_assert (!edge->aux);
	      update_edge_key (&edge_heap, edge);
	    }
	  if (edge->speculative)
	    has_speculative = true;
	}
      if (has_speculative)
	for (edge = node->callees; edge; edge = next)
	  {
	    next = edge->next_callee;
	    if (edge->speculative
		&& !speculation_useful_p (edge, edge->aux != NULL))
	      {
		cgraph_edge::resolve_speculation (edge);
		update = true;
	      }
	  }
      if (update)
	{
	  struct cgraph_node *where = node->inlined_to
				      ? node->inlined_to : node;
	  ipa_update_overall_fn_summary (where);
	  reset_edge_caches (where);
          update_caller_keys (&edge_heap, where,
			      updated_nodes, NULL);
          update_callee_keys (&edge_heap, where, NULL,
			      updated_nodes);
          bitmap_clear (updated_nodes);
	}
    }


// Source: ipa-inline.c
// Lines 1889-2016
