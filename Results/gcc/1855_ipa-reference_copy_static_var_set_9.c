copy_static_var_set (bitmap set, bool for_propagation)
{
  if (set == NULL || set == all_module_statics)
    return set;
  if (!for_propagation && set == no_module_statics)
    return set;
  bitmap_obstack *o = set->obstack;
  gcc_checking_assert (o);
  bitmap copy = BITMAP_ALLOC (o);
  bitmap_copy (copy, set);
  return copy;
}


// Source: ipa-reference.c
// Lines 366-377
