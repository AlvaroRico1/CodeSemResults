int Curl_splayremove(struct Curl_tree *t,
                     struct Curl_tree *removenode,
                     struct Curl_tree **newroot)
{
  static const struct curltime KEY_NOTUSED = {
    (time_t)-1, (unsigned int)-1
  }; /* will *NEVER* appear */
  struct Curl_tree *x;

  if(!t || !removenode)
    return 1;

  if(compare(KEY_NOTUSED, removenode->key) == 0) {
    /* Key set to NOTUSED means it is a subnode within a 'same' linked list
       and thus we can unlink it easily. */
    if(removenode->samen == removenode)
      /* A non-subnode should never be set to KEY_NOTUSED */
      return 3;

    removenode->samep->samen = removenode->samen;
    removenode->samen->samep = removenode->samep;

    /* Ensures that double-remove gets caught. */
    removenode->samen = removenode;

    *newroot = t; /* return the same root */
    return 0;
  }

  t = Curl_splay(removenode->key, t);

  /* First make sure that we got the same root node as the one we want
     to remove, as otherwise we might be trying to remove a node that
     isn't actually in the tree.

     We cannot just compare the keys here as a double remove in quick
     succession of a node with key != KEY_NOTUSED && same != NULL
     could return the same key but a different node. */
  if(t != removenode)
    return 2;

  /* Check if there is a list with identical sizes, as then we're trying to
     remove the root node of a list of nodes with identical keys. */
  x = t->samen;
  if(x != t) {
    /* 'x' is the new root node, we just make it use the root node's
       smaller/larger links */

    x->key = t->key;
    x->larger = t->larger;
    x->smaller = t->smaller;
    x->samep = t->samep;
    t->samep->samen = x;
  }
  else {
    /* Remove the root node */
    if(!t->smaller)
      x = t->larger;
    else {
      x = Curl_splay(removenode->key, t->smaller);
      x->larger = t->larger;
    }
  }

  *newroot = x; /* store new root pointer */

  return 0;
}


// Source: splay.c
// Lines 209-276
