et_root (struct et_node *node)
{
  struct et_occ *occ = node->rightmost_occ, *r;

  /* The root of the tree corresponds to the rightmost occurrence in the
     represented path.  */
  et_splay (occ);
  for (r = occ; r->next; r = r->next)
    continue;
  et_splay (r);

  return r->of;
}


// Source: et-forest.c
// Lines 755-767
