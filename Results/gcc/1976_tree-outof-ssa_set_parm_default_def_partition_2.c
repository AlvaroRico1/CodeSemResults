set_parm_default_def_partition (tree var, void *arg_)
{
  parm_default_def_partition_arg *arg = (parm_default_def_partition_arg *)arg_;
  var_map map = arg->first;
  bitmap parts = arg->second;

  if (!is_gimple_reg (var))
    return;

  tree ssa = ssa_default_def (cfun, var);
  gcc_assert (ssa);

  int version = var_to_partition (map, ssa);
  gcc_assert (version != NO_PARTITION);

  bool changed = bitmap_set_bit (parts, version);
  gcc_assert (changed);
}


// Source: tree-outof-ssa.c
// Lines 946-963
