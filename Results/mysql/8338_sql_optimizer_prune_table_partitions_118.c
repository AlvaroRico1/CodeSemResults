bool JOIN::prune_table_partitions() {
  assert(query_block->partitioned_table_count);

  for (TABLE_LIST *tbl = query_block->leaf_tables; tbl; tbl = tbl->next_leaf) {
    /*
      If tbl->embedding!=NULL that means that this table is in the inner
      part of the nested outer join, and we can't do partition pruning
      (TODO: check if this limitation can be lifted.
             This also excludes semi-joins.  Is that intentional?)
      This will try to prune non-static conditions, which can
      be used after the tables are locked.
    */
    if (!tbl->embedding) {
      Item *prune_cond =
          tbl->join_cond_optim() ? tbl->join_cond_optim() : where_cond;
      if (prune_partitions(thd, tbl->table, query_block, prune_cond))
        return true;
    }
  }

  return false;
}


// Source: sql_optimizer.cc
// Lines 2562-2583
