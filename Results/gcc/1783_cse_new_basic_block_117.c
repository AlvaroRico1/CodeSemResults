new_basic_block (void)
{
  int i;

  next_qty = 0;

  /* Invalidate cse_reg_info_table.  */
  cse_reg_info_timestamp++;

  /* Clear out hash table state for this pass.  */
  CLEAR_HARD_REG_SET (hard_regs_in_table);

  /* The per-quantity values used to be initialized here, but it is
     much faster to initialize each as it is made in `make_new_qty'.  */

  for (i = 0; i < HASH_SIZE; i++)
    {
      struct table_elt *first;

      first = table[i];
      if (first != NULL)
	{
	  struct table_elt *last = first;

	  table[i] = NULL;

	  while (last->next_same_hash != NULL)
	    last = last->next_same_hash;

	  /* Now relink this hash entire chain into
	     the free element list.  */

	  last->next_same_hash = free_element_chain;
	  free_element_chain = first;
	}


// Source: cse.c
// Lines 821-855
