kwsexec (kwset_t kws, char const *text, size_t size,
	 struct kwsmatch *kwsmatch)
{
  struct kwset const *kwset = (struct kwset *) kws;
  if (kwset->words == 1 && kwset->trans == NULL)
    {
      size_t ret = bmexec (kws, text, size);
      if (kwsmatch != NULL && ret != (size_t) -1)
	{
	  kwsmatch->index = 0;
	  kwsmatch->offset[0] = ret;
	  kwsmatch->size[0] = kwset->mind;
	}
      return ret;
    }
  else
    return cwexec(kws, text, size, kwsmatch);
}


// Source: kwset.c
// Lines 747-764
