et_new_occ (struct et_node *node)
{
  et_occ *nw = et_occurrences.allocate ();

  nw->of = node;
  nw->parent = NULL;
  nw->prev = NULL;
  nw->next = NULL;

  nw->depth = 0;
  nw->min_occ = nw;
  nw->min = 0;

  return nw;
}


// Source: et-forest.c
// Lines 444-458
