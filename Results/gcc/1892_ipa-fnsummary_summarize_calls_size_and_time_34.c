summarize_calls_size_and_time (struct cgraph_node *node,
    			       ipa_fn_summary *sum)
{
  struct cgraph_edge *e;
  for (e = node->callees; e; e = e->next_callee)
    {
      if (!e->inline_failed)
	{
	  gcc_checking_assert (!ipa_call_summaries->get (e));
	  summarize_calls_size_and_time (e->callee, sum);
	  continue;
	}
      int size = 0;
      sreal time = 0;

      estimate_edge_size_and_time (e, &size, NULL, &time,
				   vNULL, vNULL, vNULL, NULL);

      struct predicate pred = true;
      class ipa_call_summary *es = ipa_call_summaries->get (e);

      if (es->predicate)
	pred = *es->predicate;
      sum->account_size_time (size, time, pred, pred, true);
    }
  for (e = node->indirect_calls; e; e = e->next_callee)
    {
      int size = 0;
      sreal time = 0;

      estimate_edge_size_and_time (e, &size, NULL, &time,
				   vNULL, vNULL, vNULL, NULL);
      struct predicate pred = true;
      class ipa_call_summary *es = ipa_call_summaries->get (e);

      if (es->predicate)
	pred = *es->predicate;
      sum->account_size_time (size, time, pred, pred, true);
    }
}


// Source: ipa-fnsummary.c
// Lines 3156-3195
