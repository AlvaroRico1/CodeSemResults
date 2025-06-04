et_split (struct et_node *t)
{
  struct et_node *father = t->father;
  struct et_occ *r, *l, *rmost, *p_occ;

  /* Update the path represented by the splay tree.  */
  rmost = t->rightmost_occ;
  et_splay (rmost);

  for (r = rmost->next; r->prev; r = r->prev)
    continue;
  et_splay (r);

  r->prev->parent = NULL;
  p_occ = t->parent_occ;
  et_splay (p_occ);
  t->parent_occ = NULL;

  l = p_occ->prev;
  p_occ->next->parent = NULL;

  set_prev (r, l);

  et_recomp_min (r);

  et_splay (rmost);
  rmost->depth = 0;
  rmost->min = 0;

  et_occurrences.remove (p_occ);

  /* Update the tree.  */
  if (father->son == t)
    father->son = t->right;
  if (father->son == t)
    father->son = NULL;
  else
    {
      t->left->right = t->right;
      t->right->left = t->left;
    }
  t->left = t->right = NULL;
  t->father = NULL;

#ifdef DEBUG_ET
  et_check_tree_sanity (rmost);
  record_path_before (rmost);

  et_check_tree_sanity (r);
  record_path_before (r);
#endif
}


// Source: et-forest.c
// Lines 574-625
