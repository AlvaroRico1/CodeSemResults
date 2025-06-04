    REMOVE_GROUP = 1 << 2
  };
  uint possible_changes;

  if (subq_predicate->substype() == Item_subselect::SINGLEROW_SUBS) {
    if (has_limit()) return;
    possible_changes = REMOVE_ORDER;
  } else {
    assert(subq_predicate->substype() == Item_subselect::EXISTS_SUBS ||
           subq_predicate->substype() == Item_subselect::IN_SUBS ||
           subq_predicate->substype() == Item_subselect::ALL_SUBS ||
           subq_predicate->substype() == Item_subselect::ANY_SUBS);
    possible_changes = REMOVE_ORDER | REMOVE_DISTINCT | REMOVE_GROUP;
  }

  uint changelog = 0;

  if ((possible_changes & REMOVE_ORDER) && order_list.elements) {
    changelog |= REMOVE_ORDER;
    empty_order_list(this);
  }

  if ((possible_changes & REMOVE_DISTINCT) && is_distinct()) {
    changelog |= REMOVE_DISTINCT;
    remove_base_options(SELECT_DISTINCT);
  }

  /*
    Remove GROUP BY if there are no aggregate functions, no HAVING clause,
    no ROLLUP and no windowing functions.
  */

  if ((possible_changes & REMOVE_GROUP) && group_list.elements &&
      !agg_func_used() && !having_cond() && olap == UNSPECIFIED_OLAP_TYPE &&
      m_windows.elements == 0) {
    changelog |= REMOVE_GROUP;
    for (ORDER *g = group_list.first; g != nullptr; g = g->next) {
      if (g->is_item_original()) {
        Item::Cleanup_after_removal_context ctx(this);
        (*g->item)->walk(&Item::clean_up_after_removal,
                         enum_walk::SUBQUERY_POSTFIX,
                         pointer_cast<uchar *>(&ctx));
      }
    }
    group_list.clear();
    while (hidden_group_field_count-- > 0) {
      fields.pop_front();
      base_ref_items[fields.size()] = nullptr;
    }
  }


// Source: sql_resolver.cc
// Lines 4067-4116
