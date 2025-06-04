big_speedup_p (struct cgraph_edge *e)
{
  sreal unspec_time;
  sreal spec_time = estimate_edge_time (e, &unspec_time);
  sreal freq = e->sreal_frequency ();
  sreal time = compute_uninlined_call_time (e, unspec_time, freq);
  sreal inlined_time = compute_inlined_call_time (e, spec_time, freq);
  cgraph_node *caller = (e->caller->inlined_to
			 ? e->caller->inlined_to
			 : e->caller);
  int limit = opt_for_fn (caller->decl, param_inline_min_speedup);

  if ((time - inlined_time) * 100 > time * limit)
    return true;
  return false;
}


// Source: ipa-inline.c
// Lines 804-819
