check_callers (struct cgraph_node *node, void *has_hot_call)
{
  struct cgraph_edge *e;
   for (e = node->callers; e; e = e->next_caller)
     {
       if (!opt_for_fn (e->caller->decl, flag_inline_functions_called_once)
	   || !opt_for_fn (e->caller->decl, optimize))
	 return true;
       if (!can_inline_edge_p (e, true))
         return true;
       if (e->recursive_p ())
	 return true;
       if (!can_inline_edge_by_limits_p (e, true))
         return true;
       if (!(*(bool *)has_hot_call) && e->maybe_hot_p ())
	 *(bool *)has_hot_call = true;
     }
  return false;
}


// Source: ipa-inline.c
// Lines 1039-1057
