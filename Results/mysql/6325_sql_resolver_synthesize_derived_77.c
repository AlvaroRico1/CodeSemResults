TABLE_LIST *Query_block::synthesize_derived(THD *thd, Query_expression *unit,
                                            Item *join_cond, bool left_outer,
                                            bool use_inner_join) {
  char name[STRING_BUFFER_USUAL_SIZE];
  const uint i = unit->first_query_block()->select_number;
  std::snprintf(name, sizeof(name), "derived_%d_%d", select_number, i);
  char *namep = thd->mem_strdup(name);
  if (namep == nullptr) return nullptr;

  auto *const ti = new (thd->mem_root) Table_ident(unit);
  if (ti == nullptr) return nullptr;

  TABLE_LIST *derived_table =
      add_table_to_list(thd, ti, namep, 0, TL_READ, MDL_SHARED_READ);
  if (derived_table == nullptr) return nullptr;

  if (left_outer) {
    derived_table->outer_join = !use_inner_join;
    if (!unit->item->is_bool_func())
      derived_table->m_was_scalar_subquery = true;

    if (join_cond != nullptr) {
      // impossible if table subquery:
      assert(derived_table->m_was_scalar_subquery);
      if (nest_derived(thd, join_cond, join_list, derived_table))
        return nullptr;
    } else {
      // The derived table is not for a subquery in a join condition
      if (add_joined_table(derived_table)) return nullptr;
      if (nest_last_join(thd) == nullptr) return nullptr;
    }
    if (derived_table->m_was_scalar_subquery) {
      auto *const join_cond_true = new (thd->mem_root) Item_func_true();
      if (join_cond_true == nullptr) return nullptr;
      derived_table->set_join_cond(join_cond_true);
    }  // else: table subquery, the join condition is complex, made by caller.
  }

  unit->derived_table = derived_table;
  return derived_table;
}


// Source: sql_resolver.cc
// Lines 5560-5600
