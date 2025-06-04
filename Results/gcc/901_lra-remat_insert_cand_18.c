insert_cand (cand_t cand)
{
  void **entry_ptr;

  entry_ptr = htab_find_slot (cand_table, cand, INSERT);
  if (*entry_ptr == NULL)
    *entry_ptr = (void *) cand;
  return (cand_t) *entry_ptr;
}


// Source: lra-remat.c
// Lines 218-226
