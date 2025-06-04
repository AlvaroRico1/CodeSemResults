flatten_remove_node_hook (struct cgraph_node *node, void *data)
{
  if (lookup_attribute ("flatten", DECL_ATTRIBUTES (node->decl)) == NULL)
    return;

  hash_set<struct cgraph_node *> *removed
    = (hash_set<struct cgraph_node *> *) data;
  removed->add (node);
}


// Source: ipa-inline.c
// Lines 2580-2588
