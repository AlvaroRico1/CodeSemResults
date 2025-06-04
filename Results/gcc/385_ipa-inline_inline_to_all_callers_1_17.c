inline_to_all_callers_1 (struct cgraph_node *node, void *data,
			 hash_set<cgraph_node *> *callers)
{
  int *num_calls = (int *)data;
  bool callee_removed = false;

  while (node->callers && !node->inlined_to)
    {
      struct cgraph_node *caller = node->callers->caller;

      if (!can_inline_edge_p (node->callers, true)
	  || !can_inline_edge_by_limits_p (node->callers, true)
	  || node->callers->recursive_p ())
	{
	  if (dump_file)
	    fprintf (dump_file, "Uninlinable call found; giving up.\n");
	  *num_calls = 0;
	  return false;
	}

      if (dump_file)
	{
	  cgraph_node *ultimate = node->ultimate_alias_target ();
	  fprintf (dump_file,
		   "\nInlining %s size %i.\n",
		   ultimate->dump_name (),
		   ipa_size_summaries->get (ultimate)->size);
	  fprintf (dump_file,
		   " Called once from %s %i insns.\n",
		   node->callers->caller->dump_name (),
		   ipa_size_summaries->get (node->callers->caller)->size);
	}

      /* Remember which callers we inlined to, delaying updating the
	 overall summary.  */
      callers->add (node->callers->caller);
      inline_call (node->callers, true, NULL, NULL, false, &callee_removed);
      if (dump_file)
	fprintf (dump_file,
		 " Inlined into %s which now has %i size\n",
		 caller->dump_name (),
		 ipa_size_summaries->get (caller)->size);
      if (!(*num_calls)--)
	{
	  if (dump_file)
	    fprintf (dump_file, "New calls found; giving up.\n");
	  return callee_removed;
	}
      if (callee_removed)
	return true;
    }


// Source: ipa-inline.c
// Lines 2372-2422
