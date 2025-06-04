verify_node_sharing_1 (tree *tp, int *walk_subtrees, void *data)
{
  hash_set<void *> *visited = (hash_set<void *> *) data;

  if (tree_node_can_be_shared (*tp))
    {
      *walk_subtrees = false;
      return NULL;
    }

  if (visited->add (*tp))
    return *tp;

  return NULL;
}


// Source: tree-cfg.c
// Lines 5179-5193
