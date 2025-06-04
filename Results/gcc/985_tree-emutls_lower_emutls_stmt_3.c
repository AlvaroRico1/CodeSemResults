lower_emutls_stmt (gimple *stmt, struct lower_emutls_data *d)
{
  struct walk_stmt_info wi;

  d->loc = gimple_location (stmt);

  memset (&wi, 0, sizeof (wi));
  wi.info = d;
  wi.val_only = true;
  walk_gimple_op (stmt, lower_emutls_1, &wi);

  if (wi.changed)
    update_stmt (stmt);
}


// Source: tree-emutls.c
// Lines 560-573
