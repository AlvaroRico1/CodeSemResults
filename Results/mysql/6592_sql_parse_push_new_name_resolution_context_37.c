bool push_new_name_resolution_context(Parse_context *pc, TABLE_LIST *left_op,
                                      TABLE_LIST *right_op) {
  THD *thd = pc->thd;
  Name_resolution_context *on_context;
  if (!(on_context = new (thd->mem_root) Name_resolution_context)) return true;
  on_context->init();
  on_context->first_name_resolution_table =
      left_op->first_leaf_for_name_resolution();
  on_context->last_name_resolution_table =
      right_op->last_leaf_for_name_resolution();
  on_context->query_block = pc->select;
  // Other tables in FROM clause of this JOIN are not visible:
  on_context->outer_context = thd->lex->current_context()->outer_context;
  on_context->next_context = pc->select->first_context;
  pc->select->first_context = on_context;

  return thd->lex->push_context(on_context);
}


// Source: sql_parse.cc
// Lines 6053-6070
