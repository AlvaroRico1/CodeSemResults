bool Query_block::check_view_privileges(THD *thd, ulong want_privilege_first,
                                        ulong want_privilege_next) {
  ulong want_privilege = want_privilege_first;
  Internal_error_handler_holder<View_error_handler, TABLE_LIST> view_handler(
      thd, true, leaf_tables);

  for (TABLE_LIST *tl = leaf_tables; tl; tl = tl->next_leaf) {
    for (TABLE_LIST *ref = tl; ref->referencing_view;
         ref = ref->referencing_view) {
      if (check_single_table_access(thd, want_privilege, ref, false))
        return true;
    }
    want_privilege = want_privilege_next;
  }
  return false;
}


// Source: sql_resolver.cc
// Lines 1091-1106
