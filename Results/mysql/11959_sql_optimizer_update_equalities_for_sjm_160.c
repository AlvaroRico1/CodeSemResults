bool JOIN::update_equalities_for_sjm() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  List_iterator<Semijoin_mat_exec> sj_it(sjm_exec_list);
  Semijoin_mat_exec *sjm_exec;
  while ((sjm_exec = sj_it++)) {
    TABLE_LIST *const sj_nest = sjm_exec->sj_nest;

    Item *cond;
    /*
      Conditions involving SJ-inner tables are only to be found in the closest
      nest's condition, which may be an AJ nest, a LEFT JOIN nest, or the
      WHERE clause.
    */
    if (sj_nest->is_aj_nest())
      cond = sj_nest->join_cond_optim();
    else if (sj_nest->outer_join_nest())
      cond = sj_nest->outer_join_nest()->join_cond_optim();
    else
      cond = where_cond;
    if (!cond) continue;

    uchar *dummy = nullptr;
    cond = cond->compile(&Item::equality_substitution_analyzer, &dummy,
                         &Item::equality_substitution_transformer,
                         (uchar *)sj_nest);
    if (cond == nullptr) return true;

    cond->update_used_tables();

    // Loop over all primary tables that follow the materialized table
    for (uint j = sjm_exec->mat_table_index + 1; j < primary_tables; j++) {
      JOIN_TAB *const tab = best_ref[j];
      for (Key_use *keyuse = tab->position()->key;
           keyuse && keyuse->table_ref == tab->table_ref &&
           keyuse->key == tab->position()->key->key;
           keyuse++) {
        uint fieldno = 0;
        for (Item *old : sj_nest->nested_join->sj_inner_exprs) {
          if (old->real_item()->eq(keyuse->val->real_item(), false)) {
            /*
              Replace the expression selected from the subquery with the
              corresponding column of the materialized temporary table.
            */
            keyuse->val = sj_nest->nested_join->sjm.mat_fields[fieldno];
            keyuse->used_tables = keyuse->val->used_tables();
            break;
          }
          fieldno++;
        }
      }
    }
  }


// Source: sql_optimizer.cc
// Lines 4806-4857
