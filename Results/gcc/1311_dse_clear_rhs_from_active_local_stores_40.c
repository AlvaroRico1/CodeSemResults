clear_rhs_from_active_local_stores (void)
{
  insn_info_t ptr = active_local_stores;

  while (ptr)
    {
      store_info *store_info = ptr->store_rec;
      /* Skip the clobbers.  */
      while (!store_info->is_set)
	store_info = store_info->next;

      store_info->rhs = NULL;
      store_info->const_rhs = NULL;

      ptr = ptr->next_local_store;
    }


// Source: dse.c
// Lines 1228-1243
