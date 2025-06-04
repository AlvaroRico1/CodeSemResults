compute_inlined_call_time (struct cgraph_edge *edge,
			   sreal time,
			   sreal freq)
{
  cgraph_node *caller = (edge->caller->inlined_to
			 ? edge->caller->inlined_to
			 : edge->caller);
  sreal caller_time = ipa_fn_summaries->get (caller)->time;

  if (freq > 0)
    time *= freq;
  else
    time = time >> 11;

  /* This calculation should match one in ipa-inline-analysis.c
     (estimate_edge_size_and_time).  */
  time -= (sreal)ipa_call_summaries->get (edge)->call_stmt_time * freq;
  time += caller_time;
  if (time <= 0)
    time = ((sreal) 1) >> 8;
  gcc_checking_assert (time >= 0);
  return time;
}


// Source: ipa-inline.c
// Lines 749-771
