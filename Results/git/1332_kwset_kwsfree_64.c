kwsfree (kwset_t kws)
{
  struct kwset *kwset;

  kwset = (struct kwset *) kws;
  obstack_free(&kwset->obstack, NULL);
  free(kws);
}


// Source: kwset.c
// Lines 768-775
