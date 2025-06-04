mark_modified (ao_ref *ao ATTRIBUTE_UNUSED, tree vdef ATTRIBUTE_UNUSED,
	       void *data)
{
  bool *b = (bool *) data;
  *b = true;
  return true;
}


// Source: ipa-fnsummary.c
// Lines 1094-1100
