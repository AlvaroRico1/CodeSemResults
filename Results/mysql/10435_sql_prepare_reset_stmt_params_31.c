static void reset_stmt_params(Prepared_statement *stmt) {
  Item_param **item = stmt->param_array;
  Item_param **end = item + stmt->param_count;
  for (; item < end; ++item) {
    (**item).reset();
    (**item).sync_clones();
  }
}


// Source: sql_prepare.cc
// Lines 1841-1848
