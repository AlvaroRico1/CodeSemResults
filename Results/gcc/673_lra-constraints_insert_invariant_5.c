insert_invariant (rtx invariant_rtx)
{
  void **entry_ptr;
  invariant_t invariant;
  invariant_ptr_t invariant_ptr;

  invariant.invariant_rtx = invariant_rtx;
  entry_ptr = htab_find_slot (invariant_table, &invariant, INSERT);
  if (*entry_ptr == NULL)
    {
      invariant_ptr = invariants_pool->allocate ();
      invariant_ptr->invariant_rtx = invariant_rtx;
      invariant_ptr->insn = NULL;
      invariants.safe_push (invariant_ptr);
      *entry_ptr = (void *) invariant_ptr;
    }
  return (invariant_ptr_t) *entry_ptr;
}


// Source: lra-constraints.c
// Lines 5139-5156
