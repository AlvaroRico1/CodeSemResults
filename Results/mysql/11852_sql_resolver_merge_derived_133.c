bool Query_block::merge_derived(THD *thd, TABLE_LIST *derived_table) {
  DBUG_TRACE;

  if (!derived_table->is_view_or_derived() || derived_table->is_merged())
    return false;

  Query_expression *const derived_query_expression =
      derived_table->derived_query_expression();

  // A derived table must be prepared before we can merge it
  assert(derived_query_expression->is_prepared());

  LEX *const lex = parent_lex;

  // Check whether the outer query allows merged views
  if ((master_query_expression() == lex->unit && !lex->can_use_merged()) ||
      lex->can_not_use_merged())
    return false;

  /*
    @todo: The implementation of LEX::can_use_merged() currently avoids
           merging of views that are contained in other views if
           can_use_merged() returns false.
  */
  /*
    Check whether derived table is mergeable, and directives allow merging;
    priority order is:
    - ALGORITHM says MERGE or TEMPTABLE
    - hint specifies MERGE or NO_MERGE (=materialization)
    - optimizer_switch's derived_merge is ON and heuristic suggests merge
  */
  if (derived_table->algorithm == VIEW_ALGORITHM_TEMPTABLE ||
      !derived_query_expression->is_mergeable())
    return false;

  if (derived_table->algorithm == VIEW_ALGORITHM_UNDEFINED) {
    const bool merge_heuristic =
        (derived_table->is_view() || allow_merge_derived) &&
        derived_query_expression->merge_heuristic(thd->lex);
    if (!hint_table_state(thd, derived_table, DERIVED_MERGE_HINT_ENUM,
                          merge_heuristic ? OPTIMIZER_SWITCH_DERIVED_MERGE : 0))
      return false;
  }

  Query_block *const derived_query_block =
      derived_query_expression->first_query_block();
  /*
    If STRAIGHT_JOIN is specified, it is not valid to merge in a query block
    that contains semi-join nests
  */
  if ((active_options() & SELECT_STRAIGHT_JOIN) &&
      (derived_query_block->has_sj_nests || derived_query_block->has_aj_nests))
    return false;

  // Check that we have room for the merged tables in the table map:
  if (leaf_table_count + derived_query_block->leaf_table_count - 1 > MAX_TABLES)
    return false;

  derived_table->set_merged();

  DBUG_PRINT("info", ("algorithm: MERGE"));

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_derived(trace,
                                 derived_table->is_view() ? "view" : "derived");
  trace_derived.add_utf8_table(derived_table)
      .add("select#", derived_query_block->select_number)
      .add("merged", true);

  Prepared_stmt_arena_holder ps_arena_holder(thd);

  // Save offset for table number adjustment
  uint table_adjust = derived_table->tableno();

  // Set up permanent list of underlying tables of a merged view
  derived_table->merge_underlying_list = derived_query_block->get_table_list();

  /**
    A view is updatable if any underlying table is updatable.
    A view is insertable-into if all underlying tables are insertable.
    A view is not updatable nor insertable if it contains an outer join
    @see mysql_register_view()
  */
  if (derived_table->is_view()) {
    bool updatable = false;
    bool insertable = true;
    bool outer_joined = false;
    for (TABLE_LIST *tr = derived_table->merge_underlying_list; tr;
         tr = tr->next_local) {
      updatable |= tr->is_updatable();
      insertable &= tr->is_insertable();
      outer_joined |= tr->is_inner_table_of_outer_join();
    }
    updatable &= !outer_joined;
    insertable &= !outer_joined;
    if (updatable) derived_table->set_updatable();
    if (insertable) derived_table->set_insertable();
  }

  // Add a nested join object to the derived table object
  if (!(derived_table->nested_join = new (thd->mem_root) NESTED_JOIN))
    return true;

  // Merge tables from underlying query block into this join nest
  if (derived_table->merge_underlying_tables(derived_query_block))
    return true; /* purecov: inspected */

  // Replace derived table in leaf table list with underlying tables:
  for (TABLE_LIST **tl = &leaf_tables; *tl; tl = &(*tl)->next_leaf) {
    if (*tl == derived_table) {
      for (TABLE_LIST *leaf = derived_query_block->leaf_tables; leaf;
           leaf = leaf->next_leaf) {
        leaf->dep_tables <<= table_adjust;
        if (leaf->next_leaf == nullptr) {
          leaf->next_leaf = (*tl)->next_leaf;
          break;
        }
      }
      *tl = derived_query_block->leaf_tables;
      break;
    }
  }

  leaf_table_count += (derived_query_block->leaf_table_count - 1);
  derived_table_count += derived_query_block->derived_table_count;
  table_func_count += derived_query_block->table_func_count;
  materialized_derived_table_count +=
      derived_query_block->materialized_derived_table_count;
  has_sj_nests |= derived_query_block->has_sj_nests;
  has_aj_nests |= derived_query_block->has_aj_nests;
  partitioned_table_count += derived_query_block->partitioned_table_count;
  cond_count += derived_query_block->cond_count;
  between_count += derived_query_block->between_count;

  // Propagate schema table indication:
  // @todo: Add to BASE options instead
  if (derived_query_block->active_options() & OPTION_SCHEMA_TABLE)
    add_base_options(OPTION_SCHEMA_TABLE);

  // Propagate nullability for derived tables within outer joins:
  if (derived_table->is_inner_table_of_outer_join())
    propagate_nullability(&derived_table->nested_join->join_list, true);

  select_n_having_items += derived_query_block->select_n_having_items;

  // Merge the WHERE clause into the outer query block
  if (derived_table->merge_where(thd)) return true; /* purecov: inspected */

  if (derived_table->create_field_translation(thd))
    return true; /* purecov: inspected */

  // Exclude the derived table query expression from query graph.
  derived_query_expression->exclude_level();

  // Don't try to access it:
  derived_table->set_derived_query_expression((Query_expression *)1);

  // Merge subquery's name resolution contexts into parent's
  merge_contexts(derived_query_block);

  repoint_contexts_of_join_nests(derived_query_block->top_join_list);

  // Leaf tables have been shuffled, so update table numbers for them
  remap_tables(thd);

  // Update table info of referenced expressions after query block is merged
  fix_tables_after_pullout(this, derived_query_block, derived_table,
                           table_adjust,
                           derived_query_expression->m_lateral_deps);

  if (derived_query_block->is_ordered()) {
    /*
      An ORDER BY clause is moved to an outer query block
      - if the outer query block allows ordering, and
      - that refers to this view/derived table only, and
      - is not part of a UNION, and
      - may have a WHERE clause but is not grouped or aggregated and is not
        itself ordered.
     Otherwise the ORDER BY clause is ignored.

     Only SELECT statements and single-table UPDATE and DELETE statements
     allow ordering.

     Up to version 5.6 included, ORDER BY was unconditionally merged.
     Currently we only merge in the simple case above, which ensures
     backward compatibility for most reasonable use cases.

     Note that table numbers in order_list do not need updating, since
     the outer query contains only one table reference.
    */
    // LIMIT currently blocks derived table merge
    assert(!derived_query_block->has_limit());

    if ((lex->sql_command == SQLCOM_SELECT ||
         lex->sql_command == SQLCOM_UPDATE ||
         lex->sql_command == SQLCOM_DELETE) &&
        !(master_query_expression()->is_union() || is_grouped() ||
          is_distinct() || is_ordered() ||
          get_table_list()->next_local != nullptr)) {
      order_list.push_back(&derived_query_block->order_list);
      /*
        If at outer-most level (not within another derived table), ensure
        the ordering columns are marked in read_set, since columns selected
        from derived tables are not marked in initial resolving.
      */
      if (!thd->derived_tables_processing) {
        Mark_field mf(thd->mark_used_columns);
        for (ORDER *o = derived_query_block->order_list.first; o != nullptr;
             o = o->next)
          o->item[0]->walk(&Item::mark_field_in_map, enum_walk::POSTFIX,
                           pointer_cast<uchar *>(&mf));
      }


// Source: sql_resolver.cc
// Lines 3382-3594
