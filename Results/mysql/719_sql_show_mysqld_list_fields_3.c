void mysqld_list_fields(THD *thd, TABLE_LIST *table_list, const char *wild) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table: %s", table_list->table_name));

  if (open_tables_for_query(thd, table_list,
                            MYSQL_OPEN_FORCE_SHARED_HIGH_PRIO_MDL))
    return;

  if (table_list->is_view_or_derived()) {
    // Setup materialized result table so that we can read the column list
    if (table_list->resolve_derived(thd, false))
      return; /* purecov: inspected */
    if (table_list->setup_materialized_derived(thd))
      return; /* purecov: inspected */
  }
  TABLE *table = table_list->table;

  mem_root_deque<Item *> field_list(thd->mem_root);

  Field **ptr, *field;
  for (ptr = table->field; (field = *ptr); ptr++) {
    if (!wild || !wild[0] ||
        !wild_case_compare(system_charset_info, field->field_name, wild)) {
      Item *item;
      if (table_list->is_view()) {
        item = new Item_ident_for_show(field, table_list->db,
                                       table_list->table_name);
        (void)item->fix_fields(thd, nullptr);
      } else {
        item = new Item_field(field);
      }
      field_list.push_back(item);
    }
  }
  restore_record(table, s->default_values);  // Get empty record
  table->use_all_columns();
  if (thd->send_result_metadata(field_list, Protocol::SEND_DEFAULTS)) return;
  if (table_list->is_view_or_derived()) {
    close_tmp_table(table);
    free_tmp_table(table);
    table_list->table = nullptr;
  }
  my_eof(thd);
}


// Source: sql_show.cc
// Lines 1363-1406
