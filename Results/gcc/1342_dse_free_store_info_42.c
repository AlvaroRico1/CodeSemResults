free_store_info (insn_info_t insn_info)
{
  store_info *cur = insn_info->store_rec;
  while (cur)
    {
      store_info *next = cur->next;
      if (cur->is_large)
	BITMAP_FREE (cur->positions_needed.large.bmap);
      if (cur->cse_base)
	cse_store_info_pool.remove (cur);
      else
	rtx_store_info_pool.remove (cur);
      cur = next;
    }


// Source: dse.c
// Lines 732-745
