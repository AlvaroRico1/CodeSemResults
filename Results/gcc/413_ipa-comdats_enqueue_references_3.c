enqueue_references (symtab_node **first,
		    symtab_node *symbol)
{
  int i;
  struct ipa_ref *ref = NULL;

  for (i = 0; symbol->iterate_reference (i, ref); i++)
    {
      symtab_node *node = ref->referred->ultimate_alias_target ();

      /* Always keep thunks in same sections as target function.  */
      if (is_a <cgraph_node *>(node))
	node = dyn_cast <cgraph_node *> (node)->function_symbol ();
      if (!node->aux && node->definition)
	{
	   node->aux = *first;
	   *first = node;
	}
    }


// Source: ipa-comdats.c
// Lines 161-179
