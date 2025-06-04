bool Query_block::supported_correlated_scalar_subquery(THD *thd,
                                                       Item::Css_info *subquery,
                                                       Item **lifted_where) {
  // Disallow if subquery is in a JOIN clause
  if (subquery->m_location &
      Item_aggregate_type::Collect_scalar_subquery_info::L_JOIN_COND)
    return false;

  // Check that we do no have correlation inside a derived table in the
  // FROM list
  for (TABLE_LIST *tr = leaf_tables; tr != nullptr; tr = tr->next_leaf)
    if (tr->is_derived() && tr->derived_query_expression()->uncacheable)
      return false;

  // Disallow LIMIT, OFFSET
  if (has_limit()) return false;

  // Disallow window functions: transform not valid in their presence.
  if (has_windows()) return false;

  const size_t first_selected = CountHiddenFields(fields);
  if (is_implicitly_grouped()) {
    Item_sum::Collect_grouped_aggregate_info aggregates(this);
    if (fields[first_selected]->walk(&Item::collect_grouped_aggregates,
                                     enum_walk::PREFIX,
                                     pointer_cast<uchar *>(&aggregates))) {
      return true;
    }
    bool saw_count{false};
    Item_sum *cnt_item{nullptr};
    for (auto a : aggregates.list) {
      if (a->sum_func() == Item_sum::COUNT_FUNC ||
          a->sum_func() == Item_sum::COUNT_DISTINCT_FUNC) {
        saw_count = true;
        cnt_item = a;
      }
    }

    if (saw_count) {
      // The COUNT() must be the selected item, no expression involved
      if (fields[first_selected] != cnt_item) return false;
      // If we have an occurrence of COUNT() in the selected expression and
      // implicit grouping , we know that the transform can yield NULL rather
      // than 0. In such a case, we need to add a COALESCE around the replaced
      // subquery expression, i.e. COALESCE(derived.`COUNT()`, 0). This is
      // because in a LEFT JOIN inner position, a COUNT(0) can yield NULL
      // which it could not in the original subquery position.
      subquery->m_add_coalesce = true;
    }
  }

  // Only allow outer reference in the WHERE clause, check now

  // 1. select list
  for (Item *sel_expr : fields) {
    if (sel_expr->is_outer_reference()) return false;
  }

  // 2. group by clause
  if (is_grouped()) {
    for (ORDER *group = group_list.first; group != nullptr;
         group = group->next) {
      if ((*group->item)->is_outer_reference()) return false;
    }
  }

  // 3. HAVING clause
  if (having_cond() != nullptr && having_cond()->is_outer_reference())
    return false;

  // 4. ORDER BY clause
  if (is_ordered()) {
    for (ORDER *o = order_list.first; o != nullptr; o = o->next) {
      if ((*o->item)->is_outer_reference()) return false;
    }
  }

  if (m_where_cond == nullptr) {
    // We expect to find outer references (field of a FROM table of a query
    // block directly containing this subquery) in the WHERE, since all other
    // possibilities are exhausted.  But we didn't find any correlated field.
    // It may have disappeared due to ORDER BY elimination in the subquery.
    // The subquery will still be marked as using having correlated fields.
    // How to handle this?
    //  TODO.  Example:
    //  SELECT t1.a, SUM(t1.b)
    //  FROM t1
    //  WHERE t1.a = (SELECT SUM(t2.b)
    //               FROM t2 ORDER BY SUM(t2.b) + SUM(t1.b) LIMIT 1)
    //  GROUP BY t
    return false;
  }

  // Check that the WHERE clause doesn't contain an aggregate function which
  // aggregates outside this query block. We only want outer reference to
  // a field.
  Item_sum::Collect_grouped_aggregate_info aggregates(this);
  if (m_where_cond->walk(&Item::collect_grouped_aggregates, enum_walk::PREFIX,
                         pointer_cast<uchar *>(&aggregates)))
    return true;

  if (aggregates.m_outside)
    // some aggregate functions aggregate in an outer query, not supported
    return false;

  // Check that the WHERE clause doesn't contain any nested scalar subqueries
  // that are still there (correlated of a kind we couldn't handle: any nested
  // subqueries that did support transformation will already have been
  // transformed).
  Item::Collect_scalar_subquery_info subqueries;
  subqueries.m_collect_unconditionally = true;
  if (m_where_cond->walk(&Item::collect_scalar_subqueries, enum_walk::PREFIX,
                         pointer_cast<uchar *>(&subqueries)))
    return true;
  if (subqueries.m_list.size() > 0) return false;

  // Get all fields/refs referenced in the WHERE clause, and count the number
  // of correlated ones.
  List<Item> fields_or_refs;
  Item::Collect_item_fields_or_refs info{&fields_or_refs};
  if (m_where_cond->walk(&Item::collect_item_field_or_ref_processor,
                         enum_walk::PREFIX | enum_walk::POSTFIX,
                         pointer_cast<uchar *>(&info)))
    return true;

  int cnt = 0;
  List_iterator<Item> li(fields_or_refs);
  while (Item *i = li++) {
    cnt = cnt + (i->is_outer_reference() ? 1 : 0);
  }

  if (cnt == 0) {
    // We didn't find any correlated field. It may have disappeared due to
    // ORDER BY elimination in the subquery. The subquery would still be marked
    // as having correlated fields. Related case to missing WHERE above.
    //
    // TODO: We can improve these two cases by returning, presuming no
    // correlation, but we would like to improve the status of the subquery's
    // used_tables instead.
    //
    // Example: (correlated field inside ORDER BY optimized away)
    // SELECT t1.a, SUM(t1.b)
    // FROM t1
    // WHERE t1.a = (SELECT SUM(t2.b)
    //               FROM t2
    //               WHERE t2.a > 4 ORDER BY t1.b)
    // GROUP BY t1.a ORDER BY t1.a LIMIT 30;
    return false;
  }

  // Extract the predicates that must be moved out to JOIN, i.e. those AND
  // constituents which contain an outer reference, and those which shall
  // remain.
  std::vector<Item *> staying;
  List<Item> going;
  Mem_root_array<Item *> condition_parts(thd->mem_root);
  bool orig_where_modified = false;
  ExtractConditions(m_where_cond, &condition_parts);  // all elements AND'ed
  for (Item *cond_part : condition_parts) {
    // If the condition part extracted is an OR condition having correlated
    // fields, we extract top level correlated condition if possible. If not,
    // transformation cannot happen.
    if (cond_part->is_outer_reference()) {
      Item *cor_pred = nullptr;
      if (cond_part->type() == Item::COND_ITEM) {
        assert(down_cast<Item_cond *>(cond_part)->functype() ==
               Item_func::COND_OR_FUNC);
        if (extract_correlated_condition(thd, &cond_part, &cor_pred))
          return false;
        // Make a note if this extracted predicate is the same as the original
        // where condition.
        if (cond_part == m_where_cond) orig_where_modified = true;
      } else {
        cor_pred = cond_part;
        cond_part = nullptr;
      }
      if (check_predicate_and_args(cor_pred)) return false;
      going.push_back(cor_pred);
    }
    if (cond_part) staying.push_back(cond_part);
  }

  // No correlated predicates. Note that we did find some fields earlier which
  // were marked as being an "outer reference". However, it might be that the
  // expression containing this outer reference is not marked as such due to
  // some optimizations. Reject such queries for transformation (Since we
  // anyways reject queries with non-correlated operands having expressions in
  // check_predicate_and_args())
  if (going.elements == 0) return false;

  // Construct a new, reduced, WHERE clause sans the lifted predicates, which
  // will stay in the subquery
  if (staying.size() == 0) {
    m_where_cond = nullptr;
  } else {
    // If the original where condition was a disjunctive correlated predicate,
    // it would have been modified when extracting the correlated condition.
    // So, just update the used tables.
    if (orig_where_modified)
      m_where_cond->update_used_tables();
    else {
      auto *new_where = down_cast<Item_cond *>(m_where_cond);
      new_where->argument_list()->clear();
      for (Item *pred : staying) new_where->argument_list()->push_back(pred);
      m_where_cond = new_where;
      new_where->update_used_tables();
    }
    assert(!m_where_cond->is_outer_reference());
  }

  // Construct the lifted part of the WHERE condition, which will go to the
  // JOIN condition
  if (going.elements == 1) {
    *lifted_where = going.head();
  } else {
    auto cond = new (thd->mem_root) Item_cond_and(going);
    if (cond == nullptr) return true;
    cond->update_used_tables();
    *lifted_where = cond;
  }

  // there is no outer reference in this query expression/block anymore
  uncacheable &= ~UNCACHEABLE_DEPENDENT;
  master_query_expression()->uncacheable &= ~UNCACHEABLE_DEPENDENT;
  return false;
}


// Source: sql_resolver.cc
// Lines 6995-7220
