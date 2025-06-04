et_new_tree (void *data)
{
  et_node *nw = et_nodes.allocate ();

  nw->data = data;
  nw->father = NULL;
  nw->left = NULL;
  nw->right = NULL;
  nw->son = NULL;

  nw->rightmost_occ = et_new_occ (nw);
  nw->parent_occ = NULL;

  return nw;
}


// Source: et-forest.c
// Lines 463-477
