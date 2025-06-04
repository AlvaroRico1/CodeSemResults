Item *convert_charset_partition_constant(Item *item, const CHARSET_INFO *cs) {
  THD *thd = current_thd;
  Name_resolution_context *context = &thd->lex->current_query_block()->context;
  TABLE_LIST *save_list = context->table_list;
  const char *save_where = thd->where;

  item = item->safe_charset_converter(thd, cs);
  context->table_list = nullptr;
  thd->where = "convert character set partition constant";
  if (!item || item->fix_fields(thd, (Item **)nullptr)) item = nullptr;
  thd->where = save_where;
  context->table_list = save_list;
  return item;
}


// Source: sql_partition.cc
// Lines 210-223
