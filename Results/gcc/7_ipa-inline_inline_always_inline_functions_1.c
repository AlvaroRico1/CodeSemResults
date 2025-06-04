inline_always_inline_functions (struct cgraph_node *node)
{
  struct cgraph_edge *e;
  bool inlined = false;

  for (e = node->callees; e; e = e->next_callee)
    {
      struct cgraph_node *callee = e->callee->ultimate_alias_target ();
      if (!DECL_DISREGARD_INLINE_LIMITS (callee->decl))
	continue;

      if (e->recursive_p ())
	{
	  if (dump_enabled_p ())
	    dump_printf_loc (MSG_MISSED_OPTIMIZATION, e->call_stmt,
			     "  Not inlining recursive call to %C.\n",
			     e->callee);
	  e->inline_failed = CIF_RECURSIVE_INLINING;
	  continue;
	}

      if (!can_early_inline_edge_p (e))
	{
	  /* Set inlined to true if the callee is marked "always_inline" but
	     is not inlinable.  This will allow flagging an error later in
	     expand_call_inline in tree-inline.c.  */
	  if (lookup_attribute ("always_inline",
				 DECL_ATTRIBUTES (callee->decl)) != NULL)
	    inlined = true;
	  continue;
	}

      if (dump_enabled_p ())
	dump_printf_loc (MSG_OPTIMIZED_LOCATIONS, e->call_stmt,
			 "  Inlining %C into %C (always_inline).\n",
			 e->callee, e->caller);
      inline_call (e, true, NULL, NULL, false);
      inlined = true;
    }
  if (inlined)
    ipa_update_overall_fn_summary (node);

  return inlined;
}


// Source: ipa-inline.c
// Lines 2783-2826
