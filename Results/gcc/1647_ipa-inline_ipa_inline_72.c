ipa_inline (void)
{
  struct cgraph_node *node;
  int nnodes;
  struct cgraph_node **order;
  int i, j;
  int cold;
  bool remove_functions = false;

  order = XCNEWVEC (struct cgraph_node *, symtab->cgraph_count);

  if (dump_file)
    ipa_dump_fn_summaries (dump_file);

  nnodes = ipa_reverse_postorder (order);
  spec_rem = profile_count::zero ();

  FOR_EACH_FUNCTION (node)
    {
      node->aux = 0;

      /* Recompute the default reasons for inlining because they may have
	 changed during merging.  */
      if (in_lto_p)
	{
	  for (cgraph_edge *e = node->callees; e; e = e->next_callee)
	    {
	      gcc_assert (e->inline_failed);
	      initialize_inline_failed (e);
	    }
	  for (cgraph_edge *e = node->indirect_calls; e; e = e->next_callee)
	    initialize_inline_failed (e);
	}


// Source: ipa-inline.c
// Lines 2594-2626
