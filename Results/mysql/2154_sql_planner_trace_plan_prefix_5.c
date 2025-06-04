static void trace_plan_prefix(JOIN *join, uint idx, table_map excluded_tables) {
  THD *const thd = join->thd;
  Opt_trace_array plan_prefix(&thd->opt_trace, "plan_prefix");
  for (uint i = 0; i < idx; i++) {
    TABLE_LIST *const tr = join->positions[i].table->table_ref;
    if (!(tr->map() & excluded_tables)) {
      StringBuffer<32> str;
      tr->print(
          thd, &str,
          enum_query_type(QT_TO_SYSTEM_CHARSET | QT_SHOW_SELECT_NUMBER |
                          QT_NO_DEFAULT_DB | QT_DERIVED_TABLE_ONLY_ALIAS));
      plan_prefix.add_utf8(str.ptr(), str.length());
    }
  }
}


// Source: sql_planner.cc
// Lines 4634-4648
