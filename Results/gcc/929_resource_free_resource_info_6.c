free_resource_info (void)
{
  basic_block bb;

  if (target_hash_table != NULL)
    {
      int i;

      for (i = 0; i < TARGET_HASH_PRIME; ++i)
	{
	  struct target_info *ti = target_hash_table[i];

	  while (ti)
	    {
	      struct target_info *next = ti->next;
	      free (ti);
	      ti = next;
	    }


// Source: resource.c
// Lines 1232-1249
