inline_analyze_function (struct cgraph_node *node)
{
  push_cfun (DECL_STRUCT_FUNCTION (node->decl));

  if (dump_file)
    fprintf (dump_file, "\nAnalyzing function: %s\n", node->dump_name ());
  if (opt_for_fn (node->decl, optimize) && !node->thunk.thunk_p)
    inline_indirect_intraprocedural_analysis (node);
  compute_fn_summary (node, false);
  if (!optimize)
    {
      struct cgraph_edge *e;
      for (e = node->callees; e; e = e->next_callee)
	e->inline_failed = CIF_FUNCTION_NOT_OPTIMIZED;
      for (e = node->indirect_calls; e; e = e->next_callee)
	e->inline_failed = CIF_FUNCTION_NOT_OPTIMIZED;
    }


// Source: ipa-fnsummary.c
// Lines 4083-4099
