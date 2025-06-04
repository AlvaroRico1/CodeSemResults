void Query_block::remove_derived(THD *thd, TABLE_LIST *tl) {
  // Remove from leaf_tables
  materialized_derived_table_count--;
  derived_table_count--;

  TABLE_LIST **leafp = &leaf_tables;
  while (*leafp != nullptr) {
    if (*leafp == tl) {
      *leafp = (*leafp)->next_leaf;
      break;
    }
    leafp = &(*leafp)->next_leaf;
  }
  // Remove query expression from this block's set of query expressions
  Query_expression **unitp = &slave;
  while (*unitp != nullptr) {
    if (*unitp == tl->derived_query_expression()) {
      *unitp = (*unitp)->next;
      if (*unitp != nullptr) {
        (*unitp)->prev = unitp;
      }
      break;
    }
    unitp = &(*unitp)->next;
  }
  // Remove derived table's query block from global list
  Query_block **qbp = &thd->lex->all_query_blocks_list;
  while (*qbp != nullptr) {
    if (*qbp == tl->derived_query_expression()->first_query_block()) {
      *qbp = (*qbp)->link_next;
      if (*qbp != nullptr) {
        (*qbp)->link_prev = qbp;
      }
      break;
    }
    qbp = &(*qbp)->link_next;
  }
}


// Source: sql_resolver.cc
// Lines 5613-5650
