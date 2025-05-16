struct Curl_tree *Curl_splaygetbest(struct curltime i,
                                    struct Curl_tree *t,
                                    struct Curl_tree **removed)
{
  static struct curltime tv_zero = {0, 0};
  struct Curl_tree *x;

  if(!t) {
    *removed = NULL; /* none removed since there was no root */
    return NULL;
  }

  /* find smallest */
  t = Curl_splay(tv_zero, t);
  if(compare(i, t->key) < 0) {
    /* even the smallest is too big */
    *removed = NULL;
    return t;
  }

  /* FIRST! Check if there is a list with identical keys */
  x = t->samen;
  if(x != t) {
    /* there is, pick one from the list */

    /* 'x' is the new root node */

    x->key = t->key;
    x->larger = t->larger;
    x->smaller = t->smaller;
    x->samep = t->samep;
    t->samep->samen = x;

    *removed = t;
    return x; /* new root */
  }

  /* we splayed the tree to the smallest element, there is no smaller */
  x = t->larger;
  *removed = t;

  return x;
}


// Source: splay.c
// Lines 153-195
