treefails (register struct tree const *tree, struct trie const *fail,
	   struct trie *recourse)
{
  register struct tree *link;

  if (!tree)
    return;

  treefails(tree->llink, fail, recourse);
  treefails(tree->rlink, fail, recourse);

  /* Find, in the chain of fails going back to the root, the first
     node that has a descendant on the current label. */
  while (fail)
    {
      link = fail->links;
      while (link && tree->label != link->label)
	if (tree->label < link->label)
	  link = link->llink;
	else
	  link = link->rlink;
      if (link)
	{
	  tree->trie->fail = link->trie;
	  return;
	}
      fail = fail->fail;
    }

  tree->trie->fail = recourse;
}


// Source: kwset.c
// Lines 303-333
