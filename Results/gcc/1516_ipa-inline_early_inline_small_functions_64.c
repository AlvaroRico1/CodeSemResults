early_inline_small_functions (struct cgraph_node *node)
{
  struct cgraph_edge *e;
  bool inlined = false;

  for (e = node->callees; e; e = e->next_callee)
    {
      struct cgraph_node *callee = e->callee->ultimate_alias_target ();

      /* We can encounter not-yet-analyzed function during
	 early inlining on callgraphs with strongly
	 connected components.  */
      ipa_fn_summary *s = ipa_fn_summaries->get (callee);
      if (s == NULL || !s->inlinable || !e->inline_failed)
	continue;

      /* Do not consider functions not declared inline.  */
      if (!DECL_DECLARED_INLINE_P (callee->decl)
	  && !opt_for_fn (node->decl, flag_inline_small_functions)
	  && !opt_for_fn (node->decl, flag_inline_functions))
	continue;

      if (dump_enabled_p ())
	dump_printf_loc (MSG_NOTE, e->call_stmt,
			 "Considering inline candidate %C.\n",
			 callee);

      if (!can_early_inline_edge_p (e))
	continue;

      if (e->recursive_p ())
	{
	  if (dump_enabled_p ())
	    dump_printf_loc (MSG_MISSED_OPTIMIZATION, e->call_stmt,
			     "  Not inlining: recursive call.\n");
	  continue;
	}

      if (!want_early_inline_function_p (e))
	continue;

      if (dump_enabled_p ())
	dump_printf_loc (MSG_OPTIMIZED_LOCATIONS, e->call_stmt,
			 " Inlining %C into %C.\n",
			 callee, e->caller);
      inline_call (e, true, NULL, NULL, false);
      inlined = true;
    }

  if (inlined)
    ipa_update_overall_fn_summary (node);

  return inlined;
}


// Source: ipa-inline.c
// Lines 2832-2885
