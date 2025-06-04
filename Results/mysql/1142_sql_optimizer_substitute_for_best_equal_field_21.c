Item *substitute_for_best_equal_field(THD *thd, Item *cond,
                                      COND_EQUAL *cond_equal,
                                      JOIN_TAB **table_join_idx) {
  assert(cond->is_bool_func());
  if (cond->type() == Item::COND_ITEM) {
    List<Item> *cond_list = ((Item_cond *)cond)->argument_list();

    bool and_level =
        ((Item_cond *)cond)->functype() == Item_func::COND_AND_FUNC;
    if (and_level) {
      cond_equal = &((Item_cond_and *)cond)->cond_equal;
      cond_list->disjoin((List<Item> *)&cond_equal->current_level);

      List_iterator_fast<Item_equal> it(cond_equal->current_level);
      auto cmp = [table_join_idx](Item_field *f1, Item_field *f2) {
        return compare_fields_by_table_order(f1, f2, table_join_idx);
      };
      Item_equal *item_equal;
      while ((item_equal = it++)) {
        item_equal->sort(cmp);
      }
    }

    List_iterator<Item> li(*cond_list);
    Item *item;
    while ((item = li++)) {
      Item *new_item = substitute_for_best_equal_field(thd, item, cond_equal,
                                                       table_join_idx);
      if (new_item == nullptr) return nullptr;
      /*
        This works OK with PS/SP re-execution as changes are made to
        the arguments of AND/OR items only
      */
      if (new_item != item) li.replace(new_item);
    }

    if (and_level) {
      List_iterator_fast<Item_equal> it(cond_equal->current_level);
      Item_equal *item_equal;
      while ((item_equal = it++)) {
        cond = eliminate_item_equal(thd, cond, cond_equal->upper_levels,
                                    item_equal);
        if (cond == nullptr) return nullptr;
        // This occurs when eliminate_item_equal() founds that cond is
        // always false and substitutes it with a false value.
        // Due to this, value of item_equal will be 0, so just return it.
        if (cond->type() != Item::COND_ITEM) break;
      }
    }
    if (cond->type() == Item::COND_ITEM &&
        !((Item_cond *)cond)->argument_list()->elements)
      cond = cond->val_bool() ? implicit_cast<Item *>(new Item_func_true())
                              : implicit_cast<Item *>(new Item_func_false());
  } else if (cond->type() == Item::FUNC_ITEM &&
             (down_cast<Item_func *>(cond))->functype() ==
                 Item_func::MULT_EQUAL_FUNC) {
    Item_equal *item_equal = down_cast<Item_equal *>(cond);
    item_equal->sort([table_join_idx](Item_field *f1, Item_field *f2) {
      return compare_fields_by_table_order(f1, f2, table_join_idx);
    });
    if (cond_equal && cond_equal->current_level.head() == item_equal)
      cond_equal = cond_equal->upper_levels;
    return eliminate_item_equal(thd, nullptr, cond_equal, item_equal);
  } else {
    uchar *dummy = nullptr;
    if (cond->compile(&Item::visit_all_analyzer, &dummy,
                      &Item::replace_equal_field, nullptr) == nullptr)
      return nullptr;
  }
  return cond;
}

/**
  change field = field to field = const for each found field = const in the
  and_level

  @param thd      Thread handler
  @param save_list  saved list of COND_CMP
  @param and_father father of AND op
  @param cond       Condition where fields are replaced with constant values
  @param field      The field that will be substituted
  @param value      The substitution value

  @returns false if success, true if error
*/

static bool change_cond_ref_to_const(THD *thd, I_List<COND_CMP> *save_list,
                                     Item *and_father, Item *cond, Item *field,
                                     Item *value) {
  assert(cond->real_item()->is_bool_func());
  if (cond->type() == Item::COND_ITEM) {
    Item_cond *const item_cond = down_cast<Item_cond *>(cond);
    bool and_level = item_cond->functype() == Item_func::COND_AND_FUNC;
    List_iterator<Item> li(*item_cond->argument_list());
    Item *item;
    while ((item = li++)) {
      if (change_cond_ref_to_const(thd, save_list, and_level ? cond : item,
                                   item, field, value))
        return true;
    }
    return false;
  }
  if (cond->eq_cmp_result() == Item::COND_OK)
    return false;  // Not a boolean function

  Item_bool_func2 *func = down_cast<Item_bool_func2 *>(cond);
  Item **args = func->arguments();
  Item *left_item = args[0];
  Item *right_item = args[1];
  Item_func::Functype functype = func->functype();

  if (right_item->eq(field, false) && left_item != value &&
      right_item->cmp_context == field->cmp_context &&
      (left_item->result_type() != STRING_RESULT ||
       value->result_type() != STRING_RESULT ||
       left_item->collation.collation == value->collation.collation)) {
    Item *const clone = value->clone_item();
    if (thd->is_error()) return true;

    if (clone == nullptr) return false;

    clone->collation.set(right_item->collation);
    thd->change_item_tree(args + 1, clone);
    func->update_used_tables();
    if ((functype == Item_func::EQ_FUNC || functype == Item_func::EQUAL_FUNC) &&
        and_father != cond && !left_item->const_item()) {
      cond->marker = Item::MARKER_CONST_PROPAG;
      COND_CMP *const cond_cmp = new COND_CMP(and_father, func);
      if (cond_cmp == nullptr) return true;

      save_list->push_back(cond_cmp);
    }
    if (func->set_cmp_func()) return true;
  } else if (left_item->eq(field, false) && right_item != value &&
             left_item->cmp_context == field->cmp_context &&
             (right_item->result_type() != STRING_RESULT ||
              value->result_type() != STRING_RESULT ||
              right_item->collation.collation == value->collation.collation)) {
    Item *const clone = value->clone_item();
    if (thd->is_error()) return true;

    if (clone == nullptr) return false;

    clone->collation.set(left_item->collation);
    thd->change_item_tree(args, clone);
    value = clone;
    func->update_used_tables();
    if ((functype == Item_func::EQ_FUNC || functype == Item_func::EQUAL_FUNC) &&
        and_father != cond && !right_item->const_item()) {
      args[0] = args[1];  // For easy check
      thd->change_item_tree(args + 1, value);
      cond->marker = Item::MARKER_CONST_PROPAG;
      COND_CMP *const cond_cmp = new COND_CMP(and_father, func);
      if (cond_cmp == nullptr) return true;

      save_list->push_back(cond_cmp);
    }
    if (func->set_cmp_func()) return true;
  }
  return false;
}

/**
  Propagate constant values in a condition

  @param thd        Thread handler
  @param save_list  saved list of COND_CMP
  @param and_father father of AND op
  @param cond       Condition for which constant values are propagated

  @returns false if success, true if error
*/
static bool propagate_cond_constants(THD *thd, I_List<COND_CMP> *save_list,
                                     Item *and_father, Item *cond) {
  assert(cond->real_item()->is_bool_func());
  if (cond->type() == Item::COND_ITEM) {
    Item_cond *const item_cond = down_cast<Item_cond *>(cond);
    bool and_level = item_cond->functype() == Item_func::COND_AND_FUNC;
    List_iterator_fast<Item> li(*item_cond->argument_list());
    Item *item;
    I_List<COND_CMP> save;
    while ((item = li++)) {
      if (propagate_cond_constants(thd, &save, and_level ? cond : item, item))
        return true;
    }
    if (and_level) {  // Handle other found items
      I_List_iterator<COND_CMP> cond_itr(save);
      COND_CMP *cond_cmp;
      while ((cond_cmp = cond_itr++)) {
        Item **args = cond_cmp->cmp_func->arguments();
        if (!args[0]->const_item() &&
            change_cond_ref_to_const(thd, &save, cond_cmp->and_level,
                                     cond_cmp->and_level, args[0], args[1]))
          return true;
      }
    }
  } else if (and_father != cond &&
             cond->marker != Item::MARKER_CONST_PROPAG)  // In a AND group
  {
    Item_func *func;
    if (cond->type() == Item::FUNC_ITEM &&
        (func = down_cast<Item_func *>(cond)) &&
        (func->functype() == Item_func::EQ_FUNC ||
         func->functype() == Item_func::EQUAL_FUNC)) {
      Item **args = func->arguments();
      bool left_const = args[0]->const_item();
      bool right_const = args[1]->const_item();
      if (!(left_const && right_const) &&
          args[0]->result_type() == args[1]->result_type()) {
        if (right_const) {
          Item *item = args[1];
          if (resolve_const_item(thd, &item, args[0])) return true;
          thd->change_item_tree(&args[1], item);
          func->update_used_tables();
          if (change_cond_ref_to_const(thd, save_list, and_father, and_father,
                                       args[0], args[1]))
            return true;
        } else if (left_const) {
          Item *item = args[0];
          if (resolve_const_item(thd, &item, args[1])) return true;
          thd->change_item_tree(&args[0], item);
          func->update_used_tables();
          if (change_cond_ref_to_const(thd, save_list, and_father, and_father,
                                       args[1], args[0]))
            return true;
        }
      }
    }
  }

  return false;
}

/**
  Assign each nested join structure a bit in nested_join_map.

  @param join_list     List of tables
  @param first_unused  Number of first unused bit in nested_join_map before the
                       call

  @note
    This function is called after simplify_joins(), when there are no
    redundant nested joins.
    We cannot have more nested joins in a query block than there are tables,
    so as long as the number of bits in nested_join_map is not less than the
    maximum number of tables in a query block, nested_join_map can never
    overflow.

  @return
    First unused bit in nested_join_map after the call.
*/

uint build_bitmap_for_nested_joins(mem_root_deque<TABLE_LIST *> *join_list,
                                   uint first_unused) {
  DBUG_TRACE;
  for (TABLE_LIST *table : *join_list) {
    NESTED_JOIN *nested_join;
    if ((nested_join = table->nested_join)) {
      // We should have a join condition or a semi-join condition or both
      assert((table->join_cond() != nullptr) || table->is_sj_nest());

      nested_join->nj_map = 0;
      nested_join->nj_total = 0;
      /*
        We only record nested join information for outer join nests.
        Tables belonging in semi-join nests are recorded in the
        embedding outer join nest, if one exists.
      */
      if (table->join_cond()) {
        assert(first_unused < sizeof(nested_join_map) * 8);
        nested_join->nj_map = (nested_join_map)1 << first_unused++;
        nested_join->nj_total = nested_join->join_list.size();
      } else if (table->is_sj_nest()) {
        NESTED_JOIN *const outer_nest =
            table->embedding ? table->embedding->nested_join : nullptr;
        /*
          The semi-join nest has already been counted into the table count
          for the outer join nest as one table, so subtract 1 from the
          table count.
        */
        if (outer_nest)
          outer_nest->nj_total += (nested_join->join_list.size() - 1);
      } else
        assert(false);

      first_unused =
          build_bitmap_for_nested_joins(&nested_join->join_list, first_unused);
    }
  }
  return first_unused;
}

/** Update the dependency map for the tables. */

void JOIN::update_depend_map() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  for (uint tableno = 0; tableno < tables; tableno++) {
    JOIN_TAB *const tab = best_ref[tableno];
    TABLE_REF *const ref = &tab->ref();
    table_map depend_map = 0;
    Item **item = ref->items;
    for (uint i = 0; i < ref->key_parts; i++, item++)
      depend_map |= (*item)->used_tables();
    depend_map &= ~PSEUDO_TABLE_BITS;
    ref->depend_map = depend_map;
    for (JOIN_TAB **tab2 = map2table; depend_map; tab2++, depend_map >>= 1) {
      if (depend_map & 1) ref->depend_map |= (*tab2)->ref().depend_map;
    }
  }
}

/** Update the dependency map for the sort order. */

void JOIN::update_depend_map(ORDER *order) {
  DBUG_TRACE;
  for (; order; order = order->next) {
    table_map depend_map;
    order->item[0]->update_used_tables();
    order->depend_map = depend_map =
        order->item[0]->used_tables() & ~INNER_TABLE_BIT;
    order->used = 0;
    // Not item_sum(), RAND() and no reference to table outside of sub select
    if (!(order->depend_map & (OUTER_REF_TABLE_BIT | RAND_TABLE_BIT)) &&
        !order->item[0]->has_aggregation()) {
      for (JOIN_TAB **tab = map2table; depend_map; tab++, depend_map >>= 1) {
        if (depend_map & 1) order->depend_map |= (*tab)->ref().depend_map;
      }
    }
  }
}

/**
  Update equalities and keyuse references after semi-join materialization
  strategy is chosen.

  @details
    For each multiple equality that contains a field that is selected
    from a subquery, and that subquery is executed using a semi-join
    materialization strategy, add the corresponding column in the materialized
    temporary table to the equality.
    For each injected semi-join equality that is not converted to
    multiple equality, replace the reference to the expression selected
    from the subquery with the corresponding column in the temporary table.

    This is needed to properly reflect the equalities that involve injected
    semi-join equalities when materialization strategy is chosen.
    @see eliminate_item_equal() for how these equalities are used to generate
    correct equality predicates.

    The MaterializeScan semi-join strategy requires some additional processing:
    All primary tables after the materialized temporary table must be inspected
    for keyuse objects that point to expressions from the subquery tables.
    These references must be replaced with references to corresponding columns
    in the materialized temporary table instead. Those primary tables using
    ref access will thus be made to depend on the materialized temporary table
    instead of the subquery tables.

    Only the injected semi-join equalities need this treatment, other predicates
    will be handled correctly by the regular item substitution process.

  @return False if success, true if error
*/

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

  return false;
}

/**
  Assign set of available (prefix) tables to all tables in query block.
  Also set added tables, ie the tables added in each JOIN_TAB compared to the
  previous JOIN_TAB.
  This function must be called for every query block after the table order
  has been determined.
*/

void JOIN::set_prefix_tables() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  assert(!plan_is_const());
  /*
    The const tables are available together with the first non-const table in
    the join order.
  */
  table_map const initial_tables_map =
      const_table_map | (allow_outer_refs ? OUTER_REF_TABLE_BIT : 0);

  table_map current_tables_map = initial_tables_map;
  table_map prev_tables_map = (table_map)0;
  table_map saved_tables_map = (table_map)0;

  JOIN_TAB *last_non_sjm_tab = nullptr;  // Track the last non-sjm table

  for (uint i = const_tables; i < tables; i++) {
    JOIN_TAB *const tab = best_ref[i];
    if (!tab->table()) continue;
    /*
      Tables that are within SJ-Materialization nests cannot have their
      conditions referring to preceding non-const tables.
       - If we're looking at the first SJM table, reset current_tables_map
         to refer to only allowed tables
      @see Item_equal::get_subst_item()
      @see eliminate_item_equal()
    */
    if (sj_is_materialize_strategy(tab->get_sj_strategy())) {
      const table_map sjm_inner_tables = tab->emb_sj_nest->sj_inner_tables;
      if (!(sjm_inner_tables & current_tables_map)) {
        saved_tables_map = current_tables_map;
        current_tables_map = initial_tables_map;
        prev_tables_map = (table_map)0;
      }

      current_tables_map |= tab->table_ref->map();
      tab->set_prefix_tables(current_tables_map, prev_tables_map);
      prev_tables_map = current_tables_map;

      if (!(sjm_inner_tables & ~current_tables_map)) {
        /*
          At the end of a semi-join materialization nest,
          add non-deterministic expressions to the last table of the nest:
        */
        tab->add_prefix_tables(RAND_TABLE_BIT);

        // Restore the previous map:
        current_tables_map = saved_tables_map;
        prev_tables_map =
            last_non_sjm_tab ? last_non_sjm_tab->prefix_tables() : (table_map)0;
      }
    } else {
      last_non_sjm_tab = tab;
      current_tables_map |= tab->table_ref->map();
      tab->set_prefix_tables(current_tables_map, prev_tables_map);
      prev_tables_map = current_tables_map;
    }
  }
  /*
    Non-deterministic expressions must be added to the last table's condition.
    It solves problem with queries like SELECT * FROM t1 WHERE rand() > 0.5
  */
  if (last_non_sjm_tab != nullptr)
    last_non_sjm_tab->add_prefix_tables(RAND_TABLE_BIT);
}

/**
  Calculate best possible join order and initialize the join structure.

  @return true if success, false if error.

  The JOIN object is populated with statistics about the query,
  and a plan with table order and access method selection is made.

  The list of tables to be optimized is taken from query_block->leaf_tables.
  JOIN::where_cond is also used in the optimization.
  As a side-effect, JOIN::keyuse_array is populated with key_use information.

  Here is an overview of the logic of this function:

  - Initialize JOIN data structures and setup basic dependencies between tables.

  - Update dependencies based on join information.

  - Make key descriptions (update_ref_and_keys()).

  - Pull out semi-join tables based on table dependencies.

  - Extract tables with zero or one rows as const tables.

  - Read contents of const tables, substitute columns from these tables with
    actual data. Also keep track of empty tables vs. one-row tables.

  - After const table extraction based on row count, more tables may
    have become functionally dependent. Extract these as const tables.

  - Add new sargable predicates based on retrieved const values.

  - Calculate number of rows to be retrieved from each table.

  - Calculate cost of potential semi-join materializations.

  - Calculate best possible join order based on available statistics.

  - Fill in remaining information for the generated join order.
*/

bool JOIN::make_join_plan() {
  DBUG_TRACE;

  SARGABLE_PARAM *sargables = nullptr;

  Opt_trace_context *const trace = &thd->opt_trace;

  if (init_planner_arrays())  // Create and initialize the arrays
    return true;

  // Outer join dependencies were initialized above, now complete the analysis.
  if (query_block->outer_join || query_block->is_recursive()) {
    if (propagate_dependencies()) {
      /*
        Catch illegal join order.
        SQL2011 forbids:
        WITH RECURSIVE rec AS (
        ... UNION ALL SELECT ... FROM tbl LEFT JOIN rec ON...)c...
        MySQL also forbids the same query with STRAIGHT_JOIN instead of LEFT
        JOIN, because the algorithm of with-recursive imposes that "rec" be
        first in plan, i.e. "tbl" depends on "rec", but STRAIGHT_JOIN imposes
        the opposite dependency.
      */
      assert(query_block->is_recursive());
      my_error(ER_CTE_RECURSIVE_FORBIDDEN_JOIN_ORDER, MYF(0),
               query_block->recursive_reference->alias);
      return true;
    }
    init_key_dependencies();
  }

  if (unlikely(trace->is_started()))
    trace_table_dependencies(trace, join_tab, primary_tables);

  // Build the key access information, which is the basis for ref access.
  if (where_cond || query_block->outer_join) {
    if (update_ref_and_keys(thd, &keyuse_array, join_tab, tables, where_cond,
                            ~query_block->outer_join, query_block, &sargables))
      return true;
  }

  /*
    Pull out semi-join tables based on dependencies. Dependencies are valid
    throughout the lifetime of a query, so this operation can be performed
    on the first optimization only.
  */
  if (!query_block->sj_pullout_done && !query_block->sj_nests.empty() &&
      pull_out_semijoin_tables(this))
    return true;

  query_block->sj_pullout_done = true;
  const uint sj_nests = query_block->sj_nests.size();  // Changed by pull-out

  if (!(query_block->active_options() & OPTION_NO_CONST_TABLES)) {
    // Detect tables that are const (0 or 1 row) and read their contents.
    if (extract_const_tables()) return true;

    // Detect tables that are functionally dependent on const values.
    if (extract_func_dependent_tables()) return true;
  }
  // Possibly able to create more sargable predicates from const rows.
  if (const_tables && sargables) update_sargable_from_const(sargables);

  // Make a first estimate of the fanout for each table in the query block.
  if (estimate_rowcount()) return true;

  /*
    Apply join order hints, with the exception of
    JOIN_FIXED_ORDER and STRAIGHT_JOIN.
  */
  if (query_block->opt_hints_qb &&
      !(query_block->active_options() & SELECT_STRAIGHT_JOIN))
    query_block->opt_hints_qb->apply_join_order_hints(this);

  if (sj_nests) {
    set_semijoin_embedding();
    query_block->update_semijoin_strategies(thd);
  }

  if (!plan_is_const()) optimize_keyuse();

  allow_outer_refs = true;

  if (sj_nests && optimize_semijoin_nests_for_materialization(this))
    return true;

  // Choose the table order based on analysis done so far.
  if (Optimize_table_order(thd, this, nullptr).choose_table_order())
    return true;

  DBUG_EXECUTE_IF("bug13820776_1", thd->killed = THD::KILL_QUERY;);
  if (thd->killed || thd->is_error()) return true;

  // If this is a subquery, decide between In-to-exists and materialization
  if (query_expression()->item && decide_subquery_strategy()) return true;

  refine_best_rowcount();

  if (!(thd->variables.option_bits & OPTION_BIG_SELECTS) &&
      best_read > (double)thd->variables.max_join_size &&
      !thd->lex->is_explain()) { /* purecov: inspected */
    my_error(ER_TOO_BIG_SELECT, MYF(0));
    error = -1;
    return true;
  }

  positions = nullptr;  // But keep best_positions for get_best_combination

  // Generate an execution plan from the found optimal join order.
  if (get_best_combination()) return true;

  // Cleanup after update_ref_and_keys has added keys for derived tables.
  if (query_block->materialized_derived_table_count) finalize_derived_keys();

  // No need for this struct after new JOIN_TAB array is set up.
  best_positions = nullptr;

  // Some called function may still set error status unnoticed
  if (thd->is_error()) return true;

  // There is at least one empty const table
  if (const_table_map != found_const_table_map)
    zero_result_cause = "no matching row in const table";

  return false;
}

/**
  Initialize scratch arrays for the join order optimization

  @returns false if success, true if error

  @note If something fails during initialization, JOIN::cleanup()
        will free anything that has been partially allocated and set up.
        Arrays are created in the execution mem_root, so they will be
        deleted automatically when the mem_root is re-initialized.
*/

bool JOIN::init_planner_arrays() {
  // Up to one extra slot per semi-join nest is needed (if materialized)
  const uint sj_nests = query_block->sj_nests.size();
  const uint table_count = query_block->leaf_table_count;

  assert(primary_tables == 0 && tables == 0);

  if (!(join_tab = alloc_jtab_array(thd, table_count))) return true;

  /*
    We add 2 cells:
    - because planning stage uses 0-termination so needs +1
    - because after get_best_combination, we don't use 0-termination but
    need +2, to host at most 2 tmp sort/group/distinct tables.
  */
  if (!(best_ref = (JOIN_TAB **)thd->alloc(
            sizeof(JOIN_TAB *) *
            (table_count + sj_nests + 2 + m_windows.elements))))
    return true;

  // sort/group tmp tables have no map
  if (!(map2table = (JOIN_TAB **)thd->alloc(sizeof(JOIN_TAB *) *
                                            (table_count + sj_nests))))
    return true;

  if (!(positions = new (thd->mem_root) POSITION[table_count])) return true;

  if (!(best_positions = new (thd->mem_root) POSITION[table_count + sj_nests]))
    return true;

  /*
    Initialize data structures for tables to be joined.
    Initialize dependencies between tables.
  */
  JOIN_TAB **best_ref_p = best_ref;
  TABLE_LIST *tl = query_block->leaf_tables;

  for (JOIN_TAB *tab = join_tab; tl; tab++, tl = tl->next_leaf, best_ref_p++) {
    *best_ref_p = tab;
    TABLE *const table = tl->table;
    tab->table_ref = tl;
    tab->set_table(table);
    const int err = tl->fetch_number_of_rows();

    // Initialize the cost model for the table
    table->init_cost_model(cost_model());

    DBUG_EXECUTE_IF("bug11747970_raise_error", {
      if (!err) {
        my_error(ER_UNKNOWN_ERROR, MYF(0));
        return true;
      }
    });

    if (err) {
      table->file->print_error(err, MYF(0));
      return true;
    }
    all_table_map |= tl->map();
    tab->set_join(this);

    if (tl->is_updated() || tl->is_deleted()) {
      // As we update or delete rows, we can't read the index
      table->no_keyread = true;
    }

    tab->dependent = tl->dep_tables;  // Initialize table dependencies
    if (query_block->is_recursive()) {
      if (query_block->recursive_reference != tl)
        // Recursive reference must go first
        tab->dependent |= query_block->recursive_reference->map();
      else {
        // Recursive reference mustn't use any index
        table->covering_keys.clear_all();
        table->keys_in_use_for_group_by.clear_all();
        table->keys_in_use_for_order_by.clear_all();
      }
    }
    if (tl->schema_table) table->file->stats.records = 2;
    table->quick_condition_rows = table->file->stats.records;

    tab->init_join_cond_ref(tl);

    if (tl->outer_join_nest()) {
      // tab belongs to a nested join, maybe to several embedding joins
      tab->embedding_map = 0;
      for (TABLE_LIST *embedding = tl->embedding; embedding;
           embedding = embedding->embedding) {
        NESTED_JOIN *const nested_join = embedding->nested_join;
        tab->embedding_map |= nested_join->nj_map;
        tab->dependent |= embedding->dep_tables;
      }
    } else if (tab->join_cond()) {
      // tab is the only inner table of an outer join
      tab->embedding_map = 0;
      for (TABLE_LIST *embedding = tl->embedding; embedding;
           embedding = embedding->embedding)
        tab->embedding_map |= embedding->nested_join->nj_map;
    }

    if (tl->is_derived() && tl->derived_query_expression()->m_lateral_deps)
      has_lateral = true;

    tables++;  // Count number of initialized tables
  }

  primary_tables = tables;
  *best_ref_p = nullptr;  // Last element of array must be NULL

  return false;
}

/**
  Propagate dependencies between tables due to outer join relations.

  @returns false if success, true if error

  Build transitive closure for relation 'to be dependent on'.
  This will speed up the plan search for many cases with outer joins,
  as well as allow us to catch illegal cross references.
  Warshall's algorithm is used to build the transitive closure.
  As we may restart the outer loop upto 'table_count' times, the
  complexity of the algorithm is O((number of tables)^3).
  However, most of the iterations will be shortcircuited when
  there are no dependencies to propagate.
*/

bool JOIN::propagate_dependencies() {
  for (uint i = 0; i < tables; i++) {
    if (!join_tab[i].dependent) continue;

    // Add my dependencies to other tables depending on me
    uint j;
    JOIN_TAB *tab;
    for (j = 0, tab = join_tab; j < tables; j++, tab++) {
      if (tab->dependent & join_tab[i].table_ref->map()) {
        const table_map was_dependent = tab->dependent;
        tab->dependent |= join_tab[i].dependent;
        /*
          If we change dependencies for a table we already have
          processed: Redo dependency propagation from this table.
        */
        if (i > j && tab->dependent != was_dependent) {
          i = j - 1;
          break;
        }
      }
    }
  }

  JOIN_TAB *const tab_end = join_tab + tables;
  for (JOIN_TAB *tab = join_tab; tab < tab_end; tab++) {
    if ((tab->dependent & tab->table_ref->map())) return true;
  }

  return false;
}

/**
  Extract const tables based on row counts.

  @returns false if success, true if error

  This extraction must be done for each execution.
  Tables containing exactly zero or one rows are marked as const, but
  notice the additional constraints checked below.
  Tables that are extracted have their rows read before actual execution
  starts and are placed in the beginning of the join_tab array.
  Thus, they do not take part in join order optimization process,
  which can significantly reduce the optimization time.
  The data read from these tables can also be regarded as "constant"
  throughout query execution, hence the column values can be used for
  additional constant propagation and extraction of const tables based
  on eq-ref properties.

  The tables are given the type JT_SYSTEM.
*/

bool JOIN::extract_const_tables() {
  enum enum_const_table_extraction {
    extract_no_table = 0,
    extract_empty_table = 1,
    extract_const_table = 2
  };

  JOIN_TAB *const tab_end = join_tab + tables;
  for (JOIN_TAB *tab = join_tab; tab < tab_end; tab++) {
    TABLE *const table = tab->table();
    TABLE_LIST *const tl = tab->table_ref;
    enum enum_const_table_extraction extract_method = extract_const_table;

    const bool all_partitions_pruned_away = table->all_partitions_pruned_away;

    if (tl->outer_join_nest()) {
      /*
        Table belongs to a nested join, no candidate for const table extraction.
      */
      extract_method = extract_no_table;
    } else if (tl->embedding && tl->embedding->is_sj_or_aj_nest()) {
      /*
        Table belongs to a semi-join.
        We do not currently pull out const tables from semi-join nests.
      */
      extract_method = extract_no_table;
    } else if (tab->join_cond()) {
      // tab is the only inner table of an outer join, extract empty tables
      extract_method = extract_empty_table;
    }
    switch (extract_method) {
      case extract_no_table:
        break;

      case extract_empty_table:
        // Extract tables with zero rows, but only if statistics are exact
        if ((table->file->stats.records == 0 || all_partitions_pruned_away) &&
            (table->file->ha_table_flags() & HA_STATS_RECORDS_IS_EXACT))
          mark_const_table(tab, nullptr);
        break;

      case extract_const_table:
        /*
          Extract tables with zero or one rows, but do not extract tables that
           1. are dependent upon other tables, or
           2. have no exact statistics, or
           3. are full-text searched
        */
        if ((table->s->system || table->file->stats.records <= 1 ||
             all_partitions_pruned_away) &&
            !tab->dependent &&                                              // 1
            (table->file->ha_table_flags() & HA_STATS_RECORDS_IS_EXACT) &&  // 2
            !tl->is_fulltext_searched())                                    // 3
          mark_const_table(tab, nullptr);
        break;
    }
  }

  // Read const tables (tables matching no more than 1 rows)
  if (!const_tables) return false;

  for (POSITION *p_pos = positions, *p_end = p_pos + const_tables;
       p_pos < p_end; p_pos++) {
    JOIN_TAB *const tab = p_pos->table;
    const int status = join_read_const_table(tab, p_pos);
    if (status > 0)
      return true;
    else if (status == 0) {
      found_const_table_map |= tab->table_ref->map();
      tab->table_ref->optimized_away = true;
    }
  }

  return false;
}

/**
  Extract const tables based on functional dependencies.

  @returns false if success, true if error

  This extraction must be done for each execution.

  Mark as const the tables that
   - are functionally dependent on constant values, or
   - are inner tables of an outer join and contain exactly zero or one rows

  Tables that are extracted have their rows read before actual execution
  starts and are placed in the beginning of the join_tab array, just as
  described for JOIN::extract_const_tables().

  The tables are given the type JT_CONST.
*/

bool JOIN::extract_func_dependent_tables() {
  // loop until no more const tables are found
  bool ref_changed;
  // Tables referenced by others; if they're const the others may be too.
  table_map found_ref;
  do {
  more_const_tables_found:
    ref_changed = false;
    found_ref = 0;

    // Loop over all tables that are not already determined to be const
    for (JOIN_TAB **pos = best_ref + const_tables; *pos; pos++) {
      JOIN_TAB *const tab = *pos;
      TABLE *const table = tab->table();
      TABLE_LIST *const tl = tab->table_ref;
      /*
        If equi-join condition by a key is null rejecting and after a
        substitution of a const table the key value happens to be null
        then we can state that there are no matches for this equi-join.
      */
      Key_use *keyuse = tab->keyuse();
      if (keyuse && tab->join_cond() && !tab->embedding_map) {
        /*
          When performing an outer join operation if there are no matching rows
          for the single row of the outer table all the inner tables are to be
          null complemented and thus considered as constant tables.
          Here we apply this consideration to the case of outer join operations
          with a single inner table only because the case with nested tables
          would require a more thorough analysis.
          TODO. Apply single row substitution to null complemented inner tables
          for nested outer join operations.
        */
        while (keyuse->table_ref == tl) {
          if (!(keyuse->val->used_tables() & ~const_table_map) &&
              keyuse->val->is_null() && keyuse->null_rejecting &&
              (tl->embedding == nullptr ||
               !tl->embedding->is_sj_or_aj_nest())) {
            table->set_null_row();
            table->const_table = true;
            found_const_table_map |= tl->map();
            mark_const_table(tab, keyuse);
            goto more_const_tables_found;
          }
          keyuse++;
        }
      }

      if (tab->dependent)  // If dependent on some table
      {
        // All dependent tables must be const
        if (tab->dependent & ~const_table_map) {
          found_ref |= tab->dependent;
          continue;
        }
        /*
          Mark a dependent table as constant if
           1. it has exactly zero or one rows (it is a system table), and
           2. it is not within a nested outer join, and
           3. it does not have an expensive outer join condition.
              This is because we have to determine whether an outer-joined table
              has a real row or a null-extended row in the optimizer phase.
              We have no possibility to evaluate its join condition at
              execution time, when it is marked as a system table.
        */
        if (table->file->stats.records <= 1L &&                             // 1
            (table->file->ha_table_flags() & HA_STATS_RECORDS_IS_EXACT) &&  // 1
            !tl->outer_join_nest() &&                                       // 2
            !(tab->join_cond() && tab->join_cond()->is_expensive()))        // 3
        {  // system table
          mark_const_table(tab, nullptr);
          const int status =
              join_read_const_table(tab, positions + const_tables - 1);
          if (status > 0)
            return true;
          else if (status == 0)
            found_const_table_map |= tl->map();
          continue;
        }
      }

      // Check if table can be read by key or table only uses const refs

      if ((keyuse = tab->keyuse())) {
        while (keyuse->table_ref == tl) {
          Key_use *const start_keyuse = keyuse;
          const uint key = keyuse->key;
          tab->keys().set_bit(key);  // QQ: remove this ?

          table_map refs = 0;
          Key_map const_ref, eq_part;
          do {
            if (keyuse->val->type() != Item::NULL_ITEM && !keyuse->optimize) {
              if (!((~found_const_table_map) & keyuse->used_tables))
                const_ref.set_bit(keyuse->keypart);
              else
                refs |= keyuse->used_tables;
              eq_part.set_bit(keyuse->keypart);
            }
            keyuse++;
          } while (keyuse->table_ref == tl && keyuse->key == key);

          /*
            Extract const tables with proper key dependencies.
            Exclude tables that
             1. are full-text searched, or
             2. are part of nested outer join, or
             3. are part of semi-join, or
             4. have an expensive outer join condition.
             5. are blocked by handler for const table optimize.
             6. are not going to be used, typically because they are streamed
                instead of materialized
                (see Query_expression::can_materialize_directly_into_result()).
          */
          if (eq_part.is_prefix(table->key_info[key].user_defined_key_parts) &&
              !tl->is_fulltext_searched() &&                              // 1
              !tl->outer_join_nest() &&                                   // 2
              !(tl->embedding && tl->embedding->is_sj_or_aj_nest()) &&    // 3
              !(tab->join_cond() && tab->join_cond()->is_expensive()) &&  // 4
              !(table->file->ha_table_flags() & HA_BLOCK_CONST_TABLE) &&  // 5
              table->is_created()) {                                      // 6
            if (table->key_info[key].flags & HA_NOSAME) {
              if (const_ref == eq_part) {  // Found everything for ref.
                ref_changed = true;
                mark_const_table(tab, start_keyuse);
                if (create_ref_for_key(this, tab, start_keyuse,
                                       found_const_table_map))
                  return true;
                const int status =
                    join_read_const_table(tab, positions + const_tables - 1);
                if (status > 0)
                  return true;
                else if (status == 0)
                  found_const_table_map |= tl->map();
                break;
              } else
                found_ref |= refs;  // Table is const if all refs are const
            } else if (const_ref == eq_part)
              tab->const_keys.set_bit(key);
          }
        }
      }
    }
  } while
      /*
        A new const table appeared, that is referenced by others, so re-check
        others:
      */
      ((const_table_map & found_ref) && ref_changed);

  return false;
}

/**
  Update info on indexes that can be used for search lookups as
  reading const tables may has added new sargable predicates.
*/

void JOIN::update_sargable_from_const(SARGABLE_PARAM *sargables) {
  for (; sargables->field; sargables++) {
    Field *const field = sargables->field;
    JOIN_TAB *const tab = field->table->reginfo.join_tab;
    Key_map possible_keys = field->key_start;
    possible_keys.intersect(field->table->keys_in_use_for_query);
    bool is_const = true;
    for (uint j = 0; j < sargables->num_values; j++)
      is_const &= sargables->arg_value[j]->const_item();
    if (is_const) {
      tab->const_keys.merge(possible_keys);
      tab->keys().merge(possible_keys);
    }
  }
}

double find_worst_seeks(const Cost_model_table *cost_model, double num_rows,
                        double table_scan_cost) {
  /*
    Set a max value for the cost of seek operations we can expect
    when using key lookup. This can't be too high as otherwise we
    are likely to use table scan.
  */
  double worst_seeks =
      min(cost_model->page_read_cost(num_rows / 10), table_scan_cost * 3);
  const double min_worst_seek = cost_model->page_read_cost(2.0);
  return std::max(worst_seeks, min_worst_seek);  // Fix for small tables
}

/**
  Estimate the number of matched rows for each joined table.
  Set up range scan for tables that have proper predicates.

  @returns false if success, true if error
*/

bool JOIN::estimate_rowcount() {
  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_array trace_records(trace, "rows_estimation");

  JOIN_TAB *const tab_end = join_tab + tables;
  for (JOIN_TAB *tab = join_tab; tab < tab_end; tab++) {
    const Cost_model_table *const cost_model = tab->table()->cost_model();
    Opt_trace_object trace_table(trace);
    trace_table.add_utf8_table(tab->table_ref);
    if (tab->type() == JT_SYSTEM || tab->type() == JT_CONST) {
      trace_table.add("rows", 1)
          .add("cost", 1)
          .add_alnum("table_type",
                     (tab->type() == JT_SYSTEM) ? "system" : "const")
          .add("empty", tab->table()->has_null_row());

      // Only one matching row and one block to read
      tab->set_records(tab->found_records = 1);
      tab->worst_seeks = cost_model->page_read_cost(1.0);
      tab->read_time = tab->worst_seeks;
      continue;
    }
    // Approximate number of found rows and cost to read them
    tab->set_records(tab->found_records = tab->table()->file->stats.records);
    const Cost_estimate table_scan_time = tab->table()->file->table_scan_cost();
    tab->read_time = table_scan_time.total_cost();

    tab->worst_seeks =
        find_worst_seeks(cost_model, tab->found_records, tab->read_time);

    /*
      Add to tab->const_keys the indexes for which all group fields or
      all select distinct fields participate in one index.
      Add to tab->skip_scan_keys indexes which can be used for skip
      scan access if no aggregates are present.
    */
    add_loose_index_scan_and_skip_scan_keys(this, tab);

    /*
      Perform range analysis if there are keys it could use (1).
      Don't do range analysis if on the inner side of an outer join (2).
      Do range analysis if on the inner side of a semi-join (3).
    */
    TABLE_LIST *const tl = tab->table_ref;
    if ((!tab->const_keys.is_clear_all() ||
         !tab->skip_scan_keys.is_clear_all()) &&                 // (1)
        (!tl->embedding ||                                       // (2)
         (tl->embedding && tl->embedding->is_sj_or_aj_nest())))  // (3)
    {
      /*
        This call fills tab->quick() with the best QUICK access method
        possible for this table, and only if it's better than table scan.
        It also fills tab->needed_reg.
      */
      ha_rows records = get_quick_record_count(thd, tab, row_limit);

      if (records == 0 && thd->is_error()) return true;

      /*
        Check for "impossible range", but make sure that we do not attempt
        to mark semi-joined tables as "const" (only semi-joined tables that
        are functionally dependent can be marked "const", and subsequently
        pulled out of their semi-join nests).
      */
      if (records == 0 && tab->table()->reginfo.impossible_range &&
          (!(tl->embedding && tl->embedding->is_sj_or_aj_nest()))) {
        /*
          Impossible WHERE condition or join condition
          In case of join cond, mark that one empty NULL row is matched.
          In case of WHERE, don't set found_const_table_map to get the
          caller to abort with a zero row result.
        */
        mark_const_table(tab, nullptr);
        tab->set_type(JT_CONST);  // Override setting made in mark_const_table()
        if (tab->join_cond()) {
          // Generate an empty row
          trace_table.add("returning_empty_null_row", true)
              .add_alnum("cause", "impossible_on_condition");
          found_const_table_map |= tl->map();
          tab->table()->set_null_row();  // All fields are NULL
        } else {
          trace_table.add("rows", 0).add_alnum("cause",
                                               "impossible_where_condition");
        }
      }
      if (records != HA_POS_ERROR) {
        tab->found_records = records;
        tab->read_time =
            tab->quick() ? tab->quick()->cost_est.total_cost() : 0.0;
      }
    } else {
      Opt_trace_object(trace, "table_scan")
          .add("rows", tab->found_records)
          .add("cost", tab->read_time);
    }
  }

  return false;
}

/**
  Set semi-join embedding join nest pointers.

  Set pointer to embedding semi-join nest for all semi-joined tables.
  This is the closest semi-join or anti-join nest.
  Note that this must be done for every table inside all semi-join nests,
  even for tables within outer join nests embedded in semi-join nests.
  A table can never be part of multiple semi-join nests, hence no
  ambiguities can ever occur.
  Note also that the pointer is not set for TABLE_LIST objects that
  are outer join nests within semi-join nests.
*/

void JOIN::set_semijoin_embedding() {
  assert(!query_block->sj_nests.empty());

  JOIN_TAB *const tab_end = join_tab + primary_tables;

  for (JOIN_TAB *tab = join_tab; tab < tab_end; tab++) {
    tab->emb_sj_nest = nullptr;
    for (TABLE_LIST *tl = tab->table_ref; tl->embedding; tl = tl->embedding) {
      if (tl->embedding->is_sj_or_aj_nest()) {
        assert(!tab->emb_sj_nest);
        tab->emb_sj_nest = tl->embedding;
        // Let the up-walk continue, to assert there's no AJ/SJ nest above.
      }
    }
  }
}

/**
  @brief Check if semijoin's compared types allow materialization.

  @param[in,out] sj_nest Semi-join nest containing information about correlated
         expressions. Set nested_join->sjm.scan_allowed to true if
         MaterializeScan strategy allowed. Set nested_join->sjm.lookup_allowed
         to true if MaterializeLookup strategy allowed

  @details
    This is a temporary fix for BUG#36752.

    There are two subquery materialization strategies for semijoin:

    1. Materialize and do index lookups in the materialized table. See
       BUG#36752 for description of restrictions we need to put on the
       compared expressions.

       In addition, since indexes are not supported for BLOB columns,
       this strategy can not be used if any of the columns in the
       materialized table will be BLOB/GEOMETRY columns.  (Note that
       also columns for non-BLOB values that may be greater in size
       than CONVERT_IF_BIGGER_TO_BLOB, will be represented as BLOB
       columns.)

    2. Materialize and then do a full scan of the materialized table.
       The same criteria as for MaterializeLookup are applied, except that
       BLOB/GEOMETRY columns are allowed.
*/

static void semijoin_types_allow_materialization(TABLE_LIST *sj_nest) {
  DBUG_TRACE;

  assert(sj_nest->nested_join->sj_outer_exprs.size() ==
         sj_nest->nested_join->sj_inner_exprs.size());

  if (sj_nest->nested_join->sj_outer_exprs.size() > MAX_REF_PARTS ||
      sj_nest->nested_join->sj_outer_exprs.size() == 0) {
    // building an index is impossible
    sj_nest->nested_join->sjm.scan_allowed = false;
    sj_nest->nested_join->sjm.lookup_allowed = false;
    return;
  }

  sj_nest->nested_join->sjm.scan_allowed = true;
  sj_nest->nested_join->sjm.lookup_allowed = true;

  bool blobs_involved = false;
  uint total_lookup_index_length = 0;
  uint max_key_length, max_key_part_length, max_key_parts;
  /*
    Maximum lengths for keys and key parts that are supported by
    the temporary table storage engine(s).
  */
  get_max_key_and_part_length(&max_key_length, &max_key_part_length,
                              &max_key_parts);
  auto it1 = sj_nest->nested_join->sj_outer_exprs.begin();
  auto it2 = sj_nest->nested_join->sj_inner_exprs.begin();
  while (it1 != sj_nest->nested_join->sj_outer_exprs.end() &&
         it2 != sj_nest->nested_join->sj_inner_exprs.end()) {
    Item *outer = *it1++;
    Item *inner = *it2++;
    assert(outer->real_item() && inner->real_item());
    if (!types_allow_materialization(outer, inner)) {
      sj_nest->nested_join->sjm.scan_allowed = false;
      sj_nest->nested_join->sjm.lookup_allowed = false;
      return;
    }
    blobs_involved |= inner->is_blob_field();

    // Calculate the index length of materialized table
    const uint lookup_index_length = get_key_length_tmp_table(inner);
    if (lookup_index_length > max_key_part_length)
      sj_nest->nested_join->sjm.lookup_allowed = false;
    total_lookup_index_length += lookup_index_length;
  }
  if (total_lookup_index_length > max_key_length)
    sj_nest->nested_join->sjm.lookup_allowed = false;

  if (blobs_involved) sj_nest->nested_join->sjm.lookup_allowed = false;

  DBUG_PRINT("info", ("semijoin_types_allow_materialization: ok, allowed"));
}

/**
  Index dive can be skipped if the following conditions are satisfied:
  F1) For a single table query:
     a) FORCE INDEX applies to a single index.
     b) No subquery is present.
     c) Fulltext Index is not involved.
     d) No GROUP-BY or DISTINCT clause.
     e) No ORDER-BY clause.

  F2) Not applicable to multi-table query.

  F3) This optimization is not applicable to EXPLAIN queries.

  @param tab   JOIN_TAB object.
  @param thd   THD object.
*/
static bool check_skip_records_in_range_qualification(JOIN_TAB *tab, THD *thd) {
  Query_block *select = thd->lex->current_query_block();
  TABLE *table = tab->table();
  return ((table->force_index &&
           table->keys_in_use_for_query.bits_set() == 1) &&     // F1.a
          select->parent_lex->is_single_level_stmt() &&         // F1.b
          !select->has_ft_funcs() &&                            // F1.c
          (!select->is_grouped() && !select->is_distinct()) &&  // F1.d
          !select->is_ordered() &&                              // F1.e
          select->join_list->size() == 1 &&                     // F2
          !thd->lex->is_explain());                             // F3
}

/*****************************************************************************
  Create JOIN_TABS, make a guess about the table types,
  Approximate how many records will be used in each table
*****************************************************************************/

/**
  Returns estimated number of rows that could be fetched by given
  access method.

  The function calls the range optimizer to estimate the cost of the
  cheapest QUICK_* index access method to scan one or several of the
  'keys' using the conditions 'select->cond'. The range optimizer
  compares several different types of 'quick select' methods (range
  scan, index merge, loose index scan) and selects the cheapest one.

  If the best index access method is cheaper than a table- and an index
  scan, then the range optimizer also constructs the corresponding
  QUICK_* object and assigns it to select->quick. In most cases this
  is the QUICK_* object used at later (optimization and execution)
  phases.

  @param thd    Session that runs the query.
  @param tab    JOIN_TAB of source table.
  @param limit  maximum number of rows to select.

  @note
    In case of valid range, a QUICK_SELECT_I object will be constructed and
    saved in select->quick.

  @return Estimated number of result rows selected from 'tab'.

  @retval HA_POS_ERROR For derived tables/views or if an error occur.
  @retval 0            If impossible query (i.e. certainly no rows will be
                       selected.)
*/
static ha_rows get_quick_record_count(THD *thd, JOIN_TAB *tab, ha_rows limit) {
  DBUG_TRACE;
  uchar buff[STACK_BUFF_ALLOC];
  if (check_stack_overrun(thd, STACK_MIN_SIZE, buff))
    return 0;  // Fatal error flag is set
  TABLE_LIST *const tl = tab->table_ref;
  tab->set_skip_records_in_range(
      check_skip_records_in_range_qualification(tab, thd));

  // Derived tables aren't filled yet, so no stats are available.
  if (!tl->uses_materialization()) {
    QUICK_SELECT_I *qck;
    Key_map keys_to_use = tab->const_keys;
    keys_to_use.merge(tab->skip_scan_keys);
    int error = test_quick_select(
        thd, keys_to_use,
        0,  // empty table_map
        limit,
        false,  // don't force quick range
        ORDER_NOT_RELEVANT, tab,
        tab->join_cond() ? tab->join_cond() : tab->join()->where_cond,
        &tab->needed_reg, &qck, tab->table()->force_index,
        tab->join()->query_block);
    tab->set_quick(qck);

    if (error == 1) return qck->records;
    if (error == -1) {
      tl->table->reginfo.impossible_range = true;
      return 0;
    }
    DBUG_PRINT("warning", ("Couldn't use record count on const keypart"));
  } else if (tl->is_table_function() || tl->materializable_is_const()) {
    tl->fetch_number_of_rows();
    return tl->table->file->stats.records;
  }
  return HA_POS_ERROR;
}

/*
  Get estimated record length for semi-join materialization temptable

  SYNOPSIS
    get_tmp_table_rec_length()
      items  IN subquery's select list.

  DESCRIPTION
    Calculate estimated record length for semi-join materialization
    temptable. It's an estimate because we don't follow every bit of
    create_tmp_table()'s logic. This isn't necessary as the return value of
    this function is used only for cost calculations.

  RETURN
    Length of the temptable record, in bytes
*/

static uint get_tmp_table_rec_length(const mem_root_deque<Item *> &items) {
  uint len = 0;
  for (Item *item : VisibleFields(items)) {
    switch (item->result_type()) {
      case REAL_RESULT:
        len += sizeof(double);
        break;
      case INT_RESULT:
        if (item->max_length >= (MY_INT32_NUM_DECIMAL_DIGITS - 1))
          len += 8;
        else
          len += 4;
        break;
      case STRING_RESULT:
        /* DATE/TIME and GEOMETRY fields have STRING_RESULT result type.  */
        if (item->is_temporal() || item->data_type() == MYSQL_TYPE_GEOMETRY)
          len += 8;
        else
          len += item->max_length;
        break;
      case DECIMAL_RESULT:
        len += 10;
        break;
      case ROW_RESULT:
      default:
        assert(0); /* purecov: deadcode */
        break;
    }
  }
  return len;
}

/**
   Writes to the optimizer trace information about dependencies between
   tables.
   @param trace  optimizer trace
   @param join_tabs  all JOIN_TABs of the join
   @param table_count how many JOIN_TABs in the 'join_tabs' array
*/
static void trace_table_dependencies(Opt_trace_context *trace,
                                     JOIN_TAB *join_tabs, uint table_count) {
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_array trace_dep(trace, "table_dependencies");
  for (uint i = 0; i < table_count; i++) {
    TABLE_LIST *table_ref = join_tabs[i].table_ref;
    Opt_trace_object trace_one_table(trace);
    trace_one_table.add_utf8_table(table_ref).add(
        "row_may_be_null", table_ref->table->is_nullable());
    const table_map map = table_ref->map();
    assert(map < (1ULL << table_count));
    for (uint j = 0; j < table_count; j++) {
      if (map & (1ULL << j)) {
        trace_one_table.add("map_bit", j);
        break;
      }
    }
    Opt_trace_array depends_on(trace, "depends_on_map_bits");
    static_assert(sizeof(table_ref->map()) <= 64,
                  "RAND_TABLE_BIT may be in join_tabs[i].dependent, so we test "
                  "all 64 bits.");
    for (uint j = 0; j < 64; j++) {
      if (join_tabs[i].dependent & (1ULL << j)) depends_on.add(j);
    }
  }
}

/**
  Add to join_tab[i]->condition() "table.field IS NOT NULL" conditions
  we've inferred from ref/eq_ref access performed.

    This function is a part of "Early NULL-values filtering for ref access"
    optimization.

    Example of this optimization:
    For query SELECT * FROM t1,t2 WHERE t2.key=t1.field @n
    and plan " any-access(t1), ref(t2.key=t1.field) " @n
    add "t1.field IS NOT NULL" to t1's table condition. @n

    Description of the optimization:

      We look through equalities choosen to perform ref/eq_ref access,
      pick equalities that have form "tbl.part_of_key = othertbl.field"
      (where othertbl is a non-const table and othertbl.field may be NULL)
      and add them to conditions on correspoding tables (othertbl in this
      example).

      Exception from that is the case when referred_tab->join != join.
      I.e. don't add NOT NULL constraints from any embedded subquery.
      Consider this query:
      @code
      SELECT A.f2 FROM t1 LEFT JOIN t2 A ON A.f2 = f1
      WHERE A.f3=(SELECT MIN(f3) FROM  t2 C WHERE A.f4 = C.f4) OR A.f3 IS NULL;
      @endcode
      Here condition A.f3 IS NOT NULL is going to be added to the WHERE
      condition of the embedding query.
      Another example:
      SELECT * FROM t10, t11 WHERE (t10.a < 10 OR t10.a IS NULL)
      AND t11.b <=> t10.b AND (t11.a = (SELECT MAX(a) FROM t12
      WHERE t12.b = t10.a ));
      Here condition t10.a IS NOT NULL is going to be added.
      In both cases addition of NOT NULL condition will erroneously reject
      some rows of the result set.
      referred_tab->join != join constraint would disallow such additions.

      This optimization doesn't affect the choices that ref, range, or join
      optimizer make. This was intentional because this was added after 4.1
      was GA.

    Implementation overview
      1. update_ref_and_keys() accumulates info about null-rejecting
         predicates in in Key_field::null_rejecting
      1.1 add_key_part saves these to Key_use.
      2. create_ref_for_key copies them to TABLE_REF.
      3. add_not_null_conds adds "x IS NOT NULL" to join_tab->m_condition of
         appropiate JOIN_TAB members.

  @returns false on success, true on error
*/

static bool add_not_null_conds(JOIN *join) {
  DBUG_TRACE;
  ASSERT_BEST_REF_IN_JOIN_ORDER(join);
  for (uint i = join->const_tables; i < join->tables; i++) {
    JOIN_TAB *const tab = join->best_ref[i];
    if ((tab->type() != JT_REF && tab->type() != JT_EQ_REF &&
         tab->type() != JT_REF_OR_NULL) ||
        tab->table()->is_nullable()) {
      continue;
    }
    for (uint keypart = 0; keypart < tab->ref().key_parts; keypart++) {
      if ((tab->ref().null_rejecting & ((key_part_map)1 << keypart)) == 0) {
        continue;
      }
      Item *const item = tab->ref().items[keypart]->real_item();
      if (item->type() != Item::FIELD_ITEM || !item->is_nullable()) continue;
      Item_field *const not_null_item = down_cast<Item_field *>(item);
      JOIN_TAB *referred_tab = not_null_item->field->table->reginfo.join_tab;
      /*
        For UPDATE queries such as:
        UPDATE t1 SET t1.f2=(SELECT MAX(t2.f4) FROM t2 WHERE t2.f3=t1.f1);
        not_null_item is the t1.f1, but it's referred_tab is 0.
      */
      if (referred_tab == nullptr || referred_tab->join() != join) continue;
      /* Skip if we already have a 'not null' predicate for 'item' */
      if (has_not_null_predicate(referred_tab->condition(), not_null_item))
        continue;
      Item *notnull = new Item_func_isnotnull(not_null_item);
      if (notnull == nullptr) return true;
      /*
        We need to do full fix_fields() call here in order to have correct
        notnull->const_item(). This is needed e.g. by test_quick_select
        when it is called from make_join_query_block after this function is
        called.
      */
      if (notnull->fix_fields(join->thd, &notnull)) return true;
      DBUG_EXECUTE("where",
                   print_where(join->thd, notnull, referred_tab->table()->alias,
                               QT_ORDINARY););
      referred_tab->and_with_condition(notnull);
    }
  }
  return false;
}

/**
  Check all existing AND'ed predicates in 'cond' for an existing
  'is not null 'not_null_item''-predicate.

  A condition consisting of multiple AND'ed terms is recursively
  decomposed in the search for the specified not null predicate.

  @param  cond           Condition to be checked.
  @param  not_null_item  The item in: 'is not null 'item'' to search for

  @return true if 'is not null 'not_null_item'' is a predicate
          in the specified 'cond'.
*/
static bool has_not_null_predicate(Item *cond, Item_field *not_null_item) {
  if (cond == nullptr) return false;
  if (cond->type() == Item::FUNC_ITEM) {
    Item_func *item_func = down_cast<Item_func *>(cond);
    const Item_func::Functype func_type = item_func->functype();
    return (func_type == Item_func::ISNOTNULL_FUNC &&
            item_func->key_item() == not_null_item);
  } else if (cond->type() == Item::COND_ITEM) {
    Item_cond *item_cond = down_cast<Item_cond *>(cond);
    if (item_cond->functype() == Item_func::COND_AND_FUNC) {
      List_iterator<Item> li(*item_cond->argument_list());
      Item *item;
      while ((item = li++)) {
        if (has_not_null_predicate(item, not_null_item)) return true;
      }
    }
  }
  return false;
}

/**
  Check if given expression only uses fields covered by index @a keyno in the
  table tbl. The expression can use any fields in any other tables.

  The expression is guaranteed not to be AND or OR - those constructs are
  handled outside of this function.

  Restrict some function types from being pushed down to storage engine:
  a) Don't push down the triggered conditions. Nested outer joins execution
     code may need to evaluate a condition several times (both triggered and
     untriggered).
     TODO: Consider cloning the triggered condition and using the copies for:
        1. push the first copy down, to have most restrictive index condition
           possible.
        2. Put the second copy into tab->m_condition.
  b) Stored functions contain a statement that might start new operations (like
     DML statements) from within the storage engine. This does not work against
     all SEs.
  c) Subqueries might contain nested subqueries and involve more tables.
     TODO: ROY: CHECK THIS
  d) Do not push down internal functions of type DD_INTERNAL_FUNC. When ICP is
     enabled, pushing internal functions to storage engine for evaluation will
     open data-dictionary tables. In InnoDB storage engine this will result in
     situation like recursive latching of same page by the same thread. To avoid
     such situation, internal functions of type DD_INTERNAL_FUNC are not pushed
  to storage engine for evaluation.

  @param  item           Expression to check
  @param  tbl            The table having the index
  @param  keyno          The index number
  @param  other_tbls_ok  true <=> Fields of other non-const tables are allowed

  @return false if No, true if Yes
*/

bool uses_index_fields_only(Item *item, TABLE *tbl, uint keyno,
                            bool other_tbls_ok) {
  // Restrictions b and c.
  if (item->has_stored_program() || item->has_subquery()) return false;

  // No table fields in const items
  if (item->const_for_execution()) return true;

  const Item::Type item_type = item->type();

  switch (item_type) {
    case Item::FUNC_ITEM: {
      Item_func *item_func = (Item_func *)item;
      const Item_func::Functype func_type = item_func->functype();

      if (func_type == Item_func::TRIG_COND_FUNC ||  // Restriction a.
          func_type == Item_func::DD_INTERNAL_FUNC)  // Restriction d.
        return false;

      /* This is a function, apply condition recursively to arguments */
      if (item_func->argument_count() > 0) {
        Item **item_end =
            (item_func->arguments()) + item_func->argument_count();
        for (Item **child = item_func->arguments(); child != item_end;
             child++) {
          if (!uses_index_fields_only(*child, tbl, keyno, other_tbls_ok))
            return false;
        }
      }
      return true;
    }
    case Item::COND_ITEM: {
      /*
        This is a AND/OR condition. Regular AND/OR clauses are handled by
        make_cond_for_index() which will chop off the part that can be
        checked with index. This code is for handling non-top-level AND/ORs,
        e.g. func(x AND y).
      */
      List_iterator<Item> li(*((Item_cond *)item)->argument_list());
      Item *cond_item;
      while ((cond_item = li++)) {
        if (!uses_index_fields_only(cond_item, tbl, keyno, other_tbls_ok))
          return false;
      }
      return true;
    }
    case Item::FIELD_ITEM: {
      const Item_field *item_field = down_cast<const Item_field *>(item);
      if (item_field->field->table != tbl) return other_tbls_ok;
      /*
        The below is probably a repetition - the first part checks the
        other two, but let's play it safe:
      */
      return item_field->field->part_of_key.is_set(keyno) &&
             item_field->field->type() != MYSQL_TYPE_GEOMETRY &&
             item_field->field->type() != MYSQL_TYPE_BLOB;
    }
    case Item::REF_ITEM:
      return uses_index_fields_only(item->real_item(), tbl, keyno,
                                    other_tbls_ok);
    default:
      return false; /* Play it safe, don't push unknown non-const items */
  }
}

/**
  Optimize semi-join nests that could be run with sj-materialization

  @param join           The join to optimize semi-join nests for

  @details
    Optimize each of the semi-join nests that can be run with
    materialization. For each of the nests, we
     - Generate the best join order for this "sub-join" and remember it;
     - Remember the sub-join execution cost (it's part of materialization
       cost);
     - Calculate other costs that will be incurred if we decide
       to use materialization strategy for this semi-join nest.

    All obtained information is saved and will be used by the main join
    optimization pass.

  @return false if successful, true if error
*/

static bool optimize_semijoin_nests_for_materialization(JOIN *join) {
  DBUG_TRACE;
  Opt_trace_context *const trace = &join->thd->opt_trace;

  for (TABLE_LIST *sj_nest : join->query_block->sj_nests) {
    /* As a precaution, reset pointers that were used in prior execution */
    sj_nest->nested_join->sjm.positions = nullptr;

    /* Calculate the cost of materialization if materialization is allowed. */
    if (sj_nest->nested_join->sj_enabled_strategies &
        OPTIMIZER_SWITCH_MATERIALIZATION) {
      /* A semi-join nest should not contain tables marked as const */
      assert(!(sj_nest->sj_inner_tables & join->const_table_map));

      Opt_trace_object trace_wrapper(trace);
      Opt_trace_object trace_sjmat(
          trace, "execution_plan_for_potential_materialization");
      Opt_trace_array trace_sjmat_steps(trace, "steps");
      /*
        Try semijoin materialization if the semijoin is classified as
        non-trivially-correlated.
      */
      if (sj_nest->nested_join->sj_corr_tables) continue;
      /*
        Check whether data types allow execution with materialization.
      */
      semijoin_types_allow_materialization(sj_nest);

      if (!sj_nest->nested_join->sjm.scan_allowed &&
          !sj_nest->nested_join->sjm.lookup_allowed)
        continue;

      if (Optimize_table_order(join->thd, join, sj_nest).choose_table_order())
        return true;
      const uint n_tables = my_count_bits(sj_nest->sj_inner_tables);
      calculate_materialization_costs(join, sj_nest, n_tables,
                                      &sj_nest->nested_join->sjm);
      /*
        Cost data is in sj_nest->nested_join->sjm. We also need to save the
        plan:
      */
      if (!(sj_nest->nested_join->sjm.positions =
                (POSITION *)join->thd->alloc(sizeof(POSITION) * n_tables)))
        return true;
      memcpy(sj_nest->nested_join->sjm.positions,
             join->best_positions + join->const_tables,
             sizeof(POSITION) * n_tables);
    }
  }
  return false;
}

/*
  Check if table's Key_use elements have an eq_ref(outer_tables) candidate

  SYNOPSIS
    find_eq_ref_candidate()
      tl                Table to be checked
      sj_inner_tables   Bitmap of inner tables. eq_ref(inner_table) doesn't
                        count.

  DESCRIPTION
    Check if table's Key_use elements have an eq_ref(outer_tables) candidate

  TODO
    Check again if it is feasible to factor common parts with constant table
    search

  RETURN
    true  - There exists an eq_ref(outer-tables) candidate
    false - Otherwise
*/

static bool find_eq_ref_candidate(TABLE_LIST *tl, table_map sj_inner_tables) {
  Key_use *keyuse = tl->table->reginfo.join_tab->keyuse();

  if (keyuse) {
    while (true) /* For each key */
    {
      const uint key = keyuse->key;
      KEY *const keyinfo = tl->table->key_info + key;
      key_part_map bound_parts = 0;
      if ((keyinfo->flags & (HA_NOSAME)) == HA_NOSAME) {
        do /* For all equalities on all key parts */
        {
          /* Check if this is "t.keypart = expr(outer_tables) */
          if (!(keyuse->used_tables & sj_inner_tables) &&
              !(keyuse->optimize & KEY_OPTIMIZE_REF_OR_NULL)) {
            /*
              Consider only if the resulting condition does not pass a NULL
              value through. Especially needed for a UNIQUE index on NULLable
              columns where a duplicate row is possible with NULL values.
            */
            if (keyuse->null_rejecting || !keyuse->val->is_nullable() ||
                !keyinfo->key_part[keyuse->keypart].field->is_nullable())
              bound_parts |= (key_part_map)1 << keyuse->keypart;
          }
          keyuse++;
        } while (keyuse->key == key && keyuse->table_ref == tl);

        if (bound_parts == LOWER_BITS(uint, keyinfo->user_defined_key_parts))
          return true;
        if (keyuse->table_ref != tl) return false;
      } else {
        do {
          keyuse++;
          if (keyuse->table_ref != tl) return false;
        } while (keyuse->key == key);
      }
    }
  }
  return false;
}

/**
  Pull tables out of semi-join nests based on functional dependencies

  @param join  The join where to do the semi-join table pullout

  @return False if successful, true if error (Out of memory)

  @details
    Pull tables out of semi-join nests based on functional dependencies,
    ie. if a table is accessed via eq_ref(outer_tables).
    The function may be called several times, the caller is responsible
    for setting up proper key information that this function acts upon.

    PRECONDITIONS
    When this function is called, the join may have several semi-join nests
    but it is guaranteed that one semi-join nest does not contain another.
    For functionally dependent tables to be pulled out, key information must
    have been calculated (see update_ref_and_keys()).

    POSTCONDITIONS
     * Tables that were pulled out are removed from the semi-join nest they
       belonged to and added to the parent join nest.
     * For these tables, the used_tables and not_null_tables fields of
       the semi-join nest they belonged to will be adjusted.
       The semi-join nest is also marked as correlated, and
       sj_corr_tables and sj_depends_on are adjusted if necessary.
     * Semi-join nests' sj_inner_tables is set equal to used_tables

    NOTE
    Table pullout may make uncorrelated subquery correlated. Consider this
    example:

     ... WHERE oe IN (SELECT it1.primary_key WHERE p(it1, it2) ... )

    here table it1 can be pulled out (we have it1.primary_key=oe which gives
    us functional dependency). Once it1 is pulled out, all references to it1
    from p(it1, it2) become references to outside of the subquery and thus
    make the subquery (i.e. its semi-join nest) correlated.
    Making the subquery (i.e. its semi-join nest) correlated prevents us from
    using Materialization or LooseScan to execute it.
*/

static bool pull_out_semijoin_tables(JOIN *join) {
  DBUG_TRACE;

  assert(!join->query_block->sj_nests.empty());

  Opt_trace_context *const trace = &join->thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_array trace_pullout(trace, "pulled_out_semijoin_tables");

  /* Try pulling out tables from each semi-join nest */
  for (auto sj_list_it = join->query_block->sj_nests.begin();
       sj_list_it != join->query_block->sj_nests.end();) {
    TABLE_LIST *sj_nest = *sj_list_it;
    if (sj_nest->is_aj_nest()) {
      ++sj_list_it;
      continue;
    }
    table_map pulled_tables = 0;
    /*
      Calculate set of tables within this semi-join nest that have
      other dependent tables. They cannot be pulled out. For example, with
      t1 SEMIJOIN (t2 LEFT JOIN t3 ON ...) ON t1.a=t2.pk,
      t2 cannot be pulled out because t3 depends on it.
    */
    table_map dep_tables = 0;
    for (TABLE_LIST *tbl : sj_nest->nested_join->join_list) {
      if (tbl->dep_tables & sj_nest->nested_join->used_tables)
        dep_tables |= tbl->dep_tables;
    }
    /*
      Find which tables we can pull out based on key dependency data.
      Note that pulling one table out can allow us to pull out some
      other tables too.
    */
    bool pulled_a_table;
    do {
      pulled_a_table = false;
      for (TABLE_LIST *tbl : sj_nest->nested_join->join_list) {
        if (tbl->table && !(pulled_tables & tbl->map()) &&
            !(dep_tables & tbl->map())) {
          if (find_eq_ref_candidate(
                  tbl, sj_nest->nested_join->used_tables & ~pulled_tables)) {
            pulled_a_table = true;
            pulled_tables |= tbl->map();
            Opt_trace_object(trace).add_utf8_table(tbl).add(
                "functionally_dependent", true);
            /*
              Pulling a table out of uncorrelated subquery in general makes
              it correlated. See the NOTE to this function.
            */
            sj_nest->nested_join->sj_corr_tables |= tbl->map();
            sj_nest->nested_join->sj_depends_on |= tbl->map();
          }
        }
      }
    } while (pulled_a_table);

    /*
      Move the pulled out TABLE_LIST elements to the parents.
    */
    sj_nest->nested_join->used_tables &= ~pulled_tables;
    sj_nest->nested_join->not_null_tables &= ~pulled_tables;

    /* sj_inner_tables is a copy of nested_join->used_tables */
    sj_nest->sj_inner_tables = sj_nest->nested_join->used_tables;

    bool remove = false;
    if (pulled_tables) {
      mem_root_deque<TABLE_LIST *> *upper_join_list =
          (sj_nest->embedding != nullptr)
              ? &sj_nest->embedding->nested_join->join_list
              : &join->query_block->top_join_list;

      Prepared_stmt_arena_holder ps_arena_holder(join->thd);

      for (auto child_li = sj_nest->nested_join->join_list.begin();
           child_li != sj_nest->nested_join->join_list.end();) {
        TABLE_LIST *tbl = *child_li;
        if (tbl->table && !(sj_nest->nested_join->used_tables & tbl->map())) {
          /*
            Pull the table up in the same way as simplify_joins() does:
            update join_list and embedding pointers but keep next[_local]
            pointers.
          */
          child_li = sj_nest->nested_join->join_list.erase(child_li);

          upper_join_list->push_back(tbl);

          tbl->join_list = upper_join_list;
          tbl->embedding = sj_nest->embedding;
        } else {
          ++child_li;
        }
      }

      /* Remove the sj-nest itself if we've removed everything from it */
      if (!sj_nest->nested_join->used_tables) {
        upper_join_list->erase(std::find(upper_join_list->begin(),
                                         upper_join_list->end(), sj_nest));
        /* Also remove it from the list of SJ-nests: */
        remove = true;
      }
    }

    if (remove) {
      sj_list_it = join->query_block->sj_nests.erase(sj_list_it);
    } else {
      ++sj_list_it;
    }
  }
  return false;
}

/* Values in optimize */
#define KEY_OPTIMIZE_EXISTS 1
#define KEY_OPTIMIZE_REF_OR_NULL 2

/**
  Merge new key definitions to old ones, remove those not used in both.

  This is called for OR between different levels.

  To be able to do 'ref_or_null' we merge a comparison of a column
  and 'column IS NULL' to one test.  This is useful for sub select queries
  that are internally transformed to something like:.

  @code
  SELECT * FROM t1 WHERE t1.key=outer_ref_field or t1.key IS NULL
  @endcode

  Key_field::null_rejecting is processed as follows: @n
  result has null_rejecting=true if it is set for both ORed references.
  for example:
  -   (t2.key = t1.field OR t2.key  =  t1.field) -> null_rejecting=true
  -   (t2.key = t1.field OR t2.key <=> t1.field) -> null_rejecting=false

  @todo
    The result of this is that we're missing some 'ref' accesses.
    OptimizerTeam: Fix this
*/

static Key_field *merge_key_fields(Key_field *start, Key_field *new_fields,
                                   Key_field *end, uint and_level) {
  if (start == new_fields) return start;  // Impossible or
  if (new_fields == end) return start;    // No new fields, skip all

  Key_field *first_free = new_fields;

  /* Mark all found fields in old array */
  for (; new_fields != end; new_fields++) {
    const Field *const new_field = new_fields->item_field->field;

    for (Key_field *old = start; old != first_free; old++) {
      const Field *const old_field = old->item_field->field;

      /*
        Check that the Field objects are the same, as we may have several
        Item_field objects pointing to the same Field:
      */
      if (old_field == new_field) {
        /*
          NOTE: below const_item() call really works as "!used_tables()", i.e.
          it can return false where it is feasible to make it return true.

          The cause is as follows: Some of the tables are already known to be
          const tables (the detection code is in JOIN::make_join_plan(),
          above the update_ref_and_keys() call), but we didn't propagate
          information about this: TABLE::const_table is not set to true, and
          Item::update_used_tables() hasn't been called for each item.
          The result of this is that we're missing some 'ref' accesses.
          TODO: OptimizerTeam: Fix this
        */
        if (!new_fields->val->const_item()) {
          /*
            If the value matches, we can use the key reference.
            If not, we keep it until we have examined all new values
          */
          if (old->val->eq(new_fields->val, old_field->binary())) {
            old->level = and_level;
            old->optimize =
                ((old->optimize & new_fields->optimize & KEY_OPTIMIZE_EXISTS) |
                 ((old->optimize | new_fields->optimize) &
                  KEY_OPTIMIZE_REF_OR_NULL));
            old->null_rejecting =
                (old->null_rejecting && new_fields->null_rejecting);
          }
        } else if (old->eq_func && new_fields->eq_func &&
                   old->val->eq_by_collation(new_fields->val,
                                             old_field->binary(),
                                             old_field->charset())) {
          old->level = and_level;
          old->optimize =
              ((old->optimize & new_fields->optimize & KEY_OPTIMIZE_EXISTS) |
               ((old->optimize | new_fields->optimize) &
                KEY_OPTIMIZE_REF_OR_NULL));
          old->null_rejecting =
              (old->null_rejecting && new_fields->null_rejecting);
        } else if (old->eq_func && new_fields->eq_func &&
                   ((old->val->const_item() && old->val->is_null()) ||
                    new_fields->val->is_null())) {
          /* field = expression OR field IS NULL */
          old->level = and_level;
          old->optimize = KEY_OPTIMIZE_REF_OR_NULL;
          /*
            Remember the NOT NULL value unless the value does not depend
            on other tables.
          */
          if (!old->val->used_tables() && old->val->is_null())
            old->val = new_fields->val;
          /* The referred expression can be NULL: */
          old->null_rejecting = false;
        } else {
          /*
            We are comparing two different const.  In this case we can't
            use a key-lookup on this so it's better to remove the value
            and let the range optimizer handle it
          */
          if (old == --first_free)  // If last item
            break;
          *old = *first_free;  // Remove old value
          old--;               // Retry this value
        }
      }
    }
  }
  /* Remove all not used items */
  for (Key_field *old = start; old != first_free;) {
    if (old->level != and_level) {  // Not used in all levels
      if (old == --first_free) break;
      *old = *first_free;  // Remove old value
      continue;
    }
    old++;
  }
  return first_free;
}

/**
  Given a field, return its index in semi-join's select list, or UINT_MAX

  @param item_field Field to be looked up in select list

  @retval =UINT_MAX Field is not from a semijoin-transformed subquery
  @retval <UINT_MAX Index in select list of subquery

  @details
  Given a field, find its table; then see if the table is within a
  semi-join nest and if the field was in select list of the subquery
  (if subquery was part of a quantified comparison predicate), or
  the field was a result of subquery decorrelation.
  If it was, then return the field's index in the select list.
  The value is used by LooseScan strategy.
*/

static uint get_semi_join_select_list_index(Item_field *item_field) {
  TABLE_LIST *emb_sj_nest = item_field->table_ref->embedding;
  if (emb_sj_nest && emb_sj_nest->is_sj_or_aj_nest()) {
    const mem_root_deque<Item *> &items =
        emb_sj_nest->nested_join->sj_inner_exprs;
    for (size_t i = 0; i < items.size(); i++) {
      const Item *sel_item = items[i];
      if (sel_item->type() == Item::FIELD_ITEM &&
          down_cast<const Item_field *>(sel_item)->field->eq(item_field->field))
        return i;
    }
  }
  return UINT_MAX;
}

/**
   @brief
   If EXPLAIN or if the --safe-updates option is enabled, add a warning that an
   index cannot be used for ref access.

   @details
   If EXPLAIN or if the --safe-updates option is enabled, add a warning for each
   index that cannot be used for ref access due to either type conversion or
   different collations on the field used for comparison

   Example type conversion (char compared to int):

   CREATE TABLE t1 (url char(1) PRIMARY KEY);
   SELECT * FROM t1 WHERE url=1;

   Example different collations (danish vs german2):

   CREATE TABLE t1 (url char(1) PRIMARY KEY) collate latin1_danish_ci;
   SELECT * FROM t1 WHERE url='1' collate latin1_german2_ci;

   @param thd                Thread for the connection that submitted the query
   @param field              Field used in comparison
   @param cant_use_index     Indexes that cannot be used for lookup
 */
static void warn_index_not_applicable(THD *thd, const Field *field,
                                      const Key_map cant_use_index) {
  Functional_index_error_handler functional_index_error_handler(field, thd);

  if (thd->lex->is_explain() ||
      thd->variables.option_bits & OPTION_SAFE_UPDATES)
    for (uint j = 0; j < field->table->s->keys; j++)
      if (cant_use_index.is_set(j))
        push_warning_printf(thd, Sql_condition::SL_WARNING,
                            ER_WARN_INDEX_NOT_APPLICABLE,
                            ER_THD(thd, ER_WARN_INDEX_NOT_APPLICABLE), "ref",
                            field->table->key_info[j].name, field->field_name);
}

/**
  Add a possible key to array of possible keys if it's usable as a key

  @param [in,out] key_fields Used as an input parameter in the sense that it is
  a pointer to a pointer to a memory area where an array of Key_field objects
  will stored. It is used as an out parameter in the sense that the pointer will
  be updated to point beyond the last Key_field written.

  @param thd                session context
  @param and_level          And level, to be stored in Key_field
  @param cond               Condition predicate
  @param item_field         Field used in comparison
  @param eq_func            True if we used =, <=> or IS NULL
  @param value              Array of values used for comparison with field
  @param num_values         Number of elements in the array of values
  @param usable_tables      Tables which can be used for key optimization
  @param sargables          IN/OUT Array of found sargable candidates.
                            Will be ignored in case eq_func is true.

  @note
    If we are doing a NOT NULL comparison on a NOT NULL field in a outer join
    table, we store this to be able to do not exists optimization later.


  @returns false if success, true if error
*/

static bool add_key_field(THD *thd, Key_field **key_fields, uint and_level,
                          Item_func *cond, Item_field *item_field, bool eq_func,
                          Item **value, uint num_values,
                          table_map usable_tables, SARGABLE_PARAM **sargables) {
  assert(cond->is_bool_func());
  assert(eq_func || sargables);
  assert(cond->functype() == Item_func::EQ_FUNC ||
         cond->functype() == Item_func::NE_FUNC ||
         cond->functype() == Item_func::GT_FUNC ||
         cond->functype() == Item_func::LT_FUNC ||
         cond->functype() == Item_func::GE_FUNC ||
         cond->functype() == Item_func::LE_FUNC ||
         cond->functype() == Item_func::MULT_EQUAL_FUNC ||
         cond->functype() == Item_func::EQUAL_FUNC ||
         cond->functype() == Item_func::LIKE_FUNC ||
         cond->functype() == Item_func::ISNULL_FUNC ||
         cond->functype() == Item_func::ISNOTNULL_FUNC ||
         cond->functype() == Item_func::BETWEEN ||
         cond->functype() == Item_func::IN_FUNC ||
         cond->functype() == Item_func::MEMBER_OF_FUNC ||
         cond->functype() == Item_func::SP_EQUALS_FUNC ||
         cond->functype() == Item_func::SP_WITHIN_FUNC ||
         cond->functype() == Item_func::SP_CONTAINS_FUNC ||
         cond->functype() == Item_func::SP_INTERSECTS_FUNC ||
         cond->functype() == Item_func::SP_DISJOINT_FUNC ||
         cond->functype() == Item_func::SP_COVERS_FUNC ||
         cond->functype() == Item_func::SP_COVEREDBY_FUNC ||
         cond->functype() == Item_func::SP_OVERLAPS_FUNC ||
         cond->functype() == Item_func::SP_TOUCHES_FUNC ||
         cond->functype() == Item_func::SP_CROSSES_FUNC);

  Field *const field = item_field->field;
  TABLE_LIST *const tl = item_field->table_ref;

  if (tl->table->reginfo.join_tab == nullptr) {
    /*
       Due to a bug in IN-to-EXISTS (grep for real_item() in item_subselect.cc
       for more info), an index over a field from an outer query might be
       considered here, which is incorrect. Their query has been fully
       optimized already so their reginfo.join_tab is NULL and we reject them.
    */
    return false;
  }

  DBUG_PRINT("info", ("add_key_field for field %s", field->field_name));
  uint exists_optimize = 0;
  if (!tl->derived_keys_ready && tl->uses_materialization() &&
      !tl->table->is_created()) {
    bool allocated;
    if (tl->update_derived_keys(thd, field, value, num_values, &allocated))
      return true;
    if (!allocated) return false;
  }
  if (!field->is_flag_set(PART_KEY_FLAG)) {
    // Don't remove column IS NULL on a LEFT JOIN table
    if (!eq_func || (*value)->type() != Item::NULL_ITEM ||
        !tl->table->is_nullable() || field->is_nullable())
      return false;  // Not a key. Skip it
    exists_optimize = KEY_OPTIMIZE_EXISTS;
    assert(num_values == 1);
  } else {
    table_map used_tables = 0;
    bool optimizable = false;
    for (uint i = 0; i < num_values; i++) {
      used_tables |= (value[i])->used_tables();
      if (!((value[i])->used_tables() & (tl->map() | RAND_TABLE_BIT)))
        optimizable = true;
    }
    if (!optimizable) return false;
    if (!(usable_tables & tl->map())) {
      if (!eq_func || (*value)->type() != Item::NULL_ITEM ||
          !tl->table->is_nullable() || field->is_nullable())
        return false;  // Can't use left join optimize
      exists_optimize = KEY_OPTIMIZE_EXISTS;
    } else {
      JOIN_TAB *stat = tl->table->reginfo.join_tab;
      Key_map possible_keys = field->key_start;
      possible_keys.intersect(tl->table->keys_in_use_for_query);
      stat[0].keys().merge(possible_keys);  // Add possible keys

      /*
        Save the following cases:
        Field op constant
        Field LIKE constant where constant doesn't start with a wildcard
        Field = field2 where field2 is in a different table
        Field op formula
        Field IS NULL
        Field IS NOT NULL
        Field BETWEEN ...
        Field IN ...
      */
      stat[0].key_dependent |= used_tables;

      bool is_const = true;
      for (uint i = 0; i < num_values; i++) {
        if (!(is_const &= value[i]->const_for_execution())) break;
      }
      if (is_const)
        stat[0].const_keys.merge(possible_keys);
      else if (!eq_func) {
        /*
          Save info to be able check whether this predicate can be
          considered as sargable for range analysis after reading const tables.
          We do not save info about equalities as update_const_equal_items
          will take care of updating info on keys from sargable equalities.
        */
        assert(sargables);
        (*sargables)--;
        /*
          The sargables and key_fields arrays share the same memory
          buffer, and grow from opposite directions, so make sure they
          don't cross.
        */
        assert(*sargables > reinterpret_cast<SARGABLE_PARAM *>(*key_fields));
        (*sargables)->field = field;
        (*sargables)->arg_value = value;
        (*sargables)->num_values = num_values;
      }
      /*
        We can't always use indexes when comparing a string index to a
        number. cmp_type() is checked to allow compare of dates to numbers.
        eq_func is NEVER true when num_values > 1
       */
      if (!eq_func) return false;

      /*
        Check if the field and value are comparable in the index.
       */
      if (!comparable_in_index(cond, field, Field::itRAW, cond->functype(),
                               *value) ||
          (field->cmp_type() == STRING_RESULT &&
           field->match_collation_to_optimize_range() &&
           field->charset() != cond->compare_collation())) {
        warn_index_not_applicable(stat->join()->thd, field, possible_keys);
        return false;
      }
    }
  }
  /*
    For the moment eq_func is always true. This slot is reserved for future
    extensions where we want to remembers other things than just eq comparisons
  */
  assert(eq_func);
  /*
    Calculate the "null rejecting" property based on the type of predicate.
    Only the <=> operator and the IS NULL and IS NOT NULL clauses may return
    true on nullable operands that have the NULL value - assuming that all
    other predicates are augmented with IS TRUE or IS FALSE truth clause,
    so that all UNKNOWN results are converted to TRUE or FALSE.

    The "null rejecting" property can be combined with the left and right
    operands to perform certain optimizations.

    If the condition has form "left.field = right.keypart" and left.field can
    be NULL, there will be no matches if left.field is NULL.
    We use null_rejecting in add_not_null_conds() to add
    'left.field IS NOT NULL' to tab->m_condition, if this is not an outer
    join. We also use it to shortcut reading rows from table "right" when
    left.field is found to be a NULL value (in RefIterator and BKA).

    It is also possible to apply optimizations to the indexed table.
    If the operation is null rejecting and there is a unique index over
    the key field, an eq_ref operation can be performed on the index, since
    we have no interest in the NULL values.

    Notice however that the null rejecting property may be cancelled out
    by the KEY_OPTIMIZE_REF_OR_NULL property: this can be set when having:

      left.field = right.keypart OR right.keypart IS NULL.
  */
  const bool null_rejecting = cond->functype() != Item_func::EQUAL_FUNC &&
                              cond->functype() != Item_func::ISNULL_FUNC &&
                              cond->functype() != Item_func::ISNOTNULL_FUNC;

  /* Store possible eq field */
  new (*key_fields) Key_field(item_field, *value, and_level, exists_optimize,
                              eq_func, null_rejecting, nullptr,
                              get_semi_join_select_list_index(item_field));
  (*key_fields)++;
  /*
    The sargables and key_fields arrays share the same memory buffer,
    and grow from opposite directions, so make sure they don't
    cross. But if sargables was NULL, eq_func had to be true and we
    don't write any sargables.
  */
  assert(sargables == nullptr ||
         *key_fields < reinterpret_cast<Key_field *>(*sargables));

  return false;
}

/**
  Add possible keys to array of possible keys originated from a simple
  predicate.

    @param  thd            session context
    @param[in,out] key_fields Pointer to add key, if usable
                           is incremented if key was stored in the array
    @param  and_level      And level, to be stored in Key_field
    @param  cond           Condition predicate
    @param  field_item     Field used in comparision
    @param  eq_func        True if we used =, <=> or IS NULL
    @param  val            Value used for comparison with field
                           Is NULL for BETWEEN and IN
    @param  num_values     Number of elements in the array of values
    @param  usable_tables  Tables which can be used for key optimization
    @param  sargables      IN/OUT Array of found sargable candidates

  @note
    If field items f1 and f2 belong to the same multiple equality and
    a key is added for f1, the the same key is added for f2.

  @returns false if success, true if error
*/

static bool add_key_equal_fields(THD *thd, Key_field **key_fields,
                                 uint and_level, Item_func *cond,
                                 Item_field *field_item, bool eq_func,
                                 Item **val, uint num_values,
                                 table_map usable_tables,
                                 SARGABLE_PARAM **sargables) {
  assert(cond->is_bool_func());

  if (add_key_field(thd, key_fields, and_level, cond, field_item, eq_func, val,
                    num_values, usable_tables, sargables))
    return true;
  Item_equal *item_equal = field_item->item_equal;
  if (item_equal == nullptr) return false;
  /*
    Add to the set of possible key values every substitution of
    the field for an equal field included into item_equal
  */
  Item_equal_iterator it(*item_equal);
  Item_field *item;
  while ((item = it++)) {
    if (!field_item->field->eq(item->field)) {
      if (add_key_field(thd, key_fields, and_level, cond, item, eq_func, val,
                        num_values, usable_tables, sargables))
        return true;
    }
  }
  return false;
}

/**
  Check if an expression is a non-outer field.

  Checks if an expression is a field and belongs to the current select.

  @param   field  Item expression to check

  @return boolean
     @retval true   the expression is a local field
     @retval false  it's something else
*/

static bool is_local_field(Item *field) {
  return field->real_item()->type() == Item::FIELD_ITEM &&
         !field->is_outer_reference() &&
         !down_cast<Item_ident *>(field)->depended_from &&
         !down_cast<Item_ident *>(field->real_item())->depended_from;
}

/**
  Check if a row constructor expression is over columns in the same query block.

  @param item_row Row expression to check.

  @return boolean
  @retval true  The expression is a local column reference.
  @retval false It's something else.
*/
static bool is_row_of_local_columns(Item_row *item_row) {
  for (uint i = 0; i < item_row->cols(); ++i)
    if (!is_local_field(item_row->element_index(i))) return false;
  return true;
}

/**
   The guts of the ref optimizer. This function, along with the other
   add_key_* functions, make up a recursive procedure that analyzes a
   condition expression (a tree of AND and OR predicates) and does
   many things.

   @param thd      session context
   @param join     The query block involving the condition.
   @param [in,out] key_fields Start of memory buffer, see below.
   @param [in,out] and_level Current 'and level', see below.
   @param cond The conditional expression to analyze.
   @param usable_tables Tables not in this bitmap will not be examined.
   @param [in,out] sargables End of memory buffer, see below.

   @returns false if success, true if error

   This documentation is the result of reverse engineering and may
   therefore not capture the full gist of the procedure, but it is
   known to do the following:

   - Populate a raw memory buffer from two directions at the same time. An
     'array' of Key_field objects fill the buffer from low to high addresses
     whilst an 'array' of SARGABLE_PARAM's fills the buffer from high to low
     addresses. At the first call to this function, it is assumed that
     key_fields points to the beginning of the buffer and sargables point to the
     end (except for a poor-mans 'null element' at the very end).

   - Update a number of properties in the JOIN_TAB's that can be used
     to find search keys (sargables).

     - JOIN_TAB::keys
     - JOIN_TAB::key_dependent
     - JOIN_TAB::const_keys (dictates if the range optimizer will be run
       later.)

   The Key_field objects are marked with something called an 'and_level', which
   does @b not correspond to their nesting depth within the expression tree. It
   is rather a tag to group conjunctions together. For instance, in the
   conditional expression

   @code
     a = 0 AND b = 0
   @endcode

   two Key_field's are produced, both having an and_level of 0.

   In an expression such as

   @code
     a = 0 AND b = 0 OR a = 1
   @endcode

   three Key_field's are produced, the first two corresponding to 'a = 0' and
   'b = 0', respectively, both with and_level 0. The third one corresponds to
   'a = 1' and has an and_level of 1.

   A separate function, merge_key_fields() performs ref access validation on
   the Key_field array on the recursice ascent. If some Key_field's cannot be
   used for ref access, the key_fields pointer is rolled back. All other
   modifications to the query plan remain.
*/
bool add_key_fields(THD *thd, JOIN *join, Key_field **key_fields,
                    uint *and_level, Item *cond, table_map usable_tables,
                    SARGABLE_PARAM **sargables) {
  assert(cond->is_bool_func());

  if (cond->type() == Item_func::COND_ITEM) {
    List_iterator_fast<Item> li(*((Item_cond *)cond)->argument_list());
    Key_field *org_key_fields = *key_fields;

    if (down_cast<Item_cond *>(cond)->functype() == Item_func::COND_AND_FUNC) {
      Item *item;
      while ((item = li++)) {
        if (add_key_fields(thd, join, key_fields, and_level, item,
                           usable_tables, sargables))
          return true;
      }
      for (; org_key_fields != *key_fields; org_key_fields++)
        org_key_fields->level = *and_level;
    } else {
      (*and_level)++;
      if (add_key_fields(thd, join, key_fields, and_level, li++, usable_tables,
                         sargables))
        return true;
      Item *item;
      while ((item = li++)) {
        Key_field *start_key_fields = *key_fields;
        (*and_level)++;
        if (add_key_fields(thd, join, key_fields, and_level, item,
                           usable_tables, sargables))
          return true;
        *key_fields = merge_key_fields(org_key_fields, start_key_fields,
                                       *key_fields, ++(*and_level));
      }
    }
    return false;
  }

  /*
    Subquery optimization: Conditions that are pushed down into subqueries
    are wrapped into Item_func_trig_cond. We process the wrapped condition
    but need to set cond_guard for Key_use elements generated from it.
  */
  if (cond->type() == Item::FUNC_ITEM &&
      down_cast<Item_func *>(cond)->functype() == Item_func::TRIG_COND_FUNC) {
    Item *const cond_arg = down_cast<Item_func *>(cond)->arguments()[0];
    if (join->group_list.empty() && join->order.empty() &&
        join->query_expression()->item &&
        join->query_expression()->item->substype() == Item_subselect::IN_SUBS &&
        !join->query_expression()->is_union()) {
      Key_field *save = *key_fields;
      if (add_key_fields(thd, join, key_fields, and_level, cond_arg,
                         usable_tables, sargables))
        return true;
      // Indicate that this ref access candidate is for subquery lookup:
      for (; save != *key_fields; save++)
        save->cond_guard = ((Item_func_trig_cond *)cond)->get_trig_var();
    }
    return false;
  }

  /* If item is of type 'field op field/constant' add it to key_fields */
  if (cond->type() != Item::FUNC_ITEM) return false;
  Item_func *const cond_func = down_cast<Item_func *>(cond);
  auto optimize = cond_func->select_optimize(thd);
  // Catch errors that might be thrown during select_optimize()
  if (thd->is_error()) return true;
  switch (optimize) {
    case Item_func::OPTIMIZE_NONE:
      break;
    case Item_func::OPTIMIZE_KEY: {
      Item **values;
      /*
        Build list of possible keys for 'a BETWEEN low AND high'.
        It is handled similar to the equivalent condition
        'a >= low AND a <= high':
      */
      if (cond_func->functype() == Item_func::BETWEEN) {
        Item_field *field_item;
        bool equal_func = false;
        uint num_values = 2;
        values = cond_func->arguments();

        bool binary_cmp =
            (values[0]->real_item()->type() == Item::FIELD_ITEM)
                ? ((Item_field *)values[0]->real_item())->field->binary()
                : true;

        /*
          Additional optimization: If 'low = high':
          Handle as if the condition was "t.key = low".
        */
        if (!((Item_func_between *)cond_func)->negated &&
            values[1]->eq(values[2], binary_cmp)) {
          equal_func = true;
          num_values = 1;
        }

        /*
          Append keys for 'field <cmp> value[]' if the
          condition is of the form::
          '<field> BETWEEN value[1] AND value[2]'
        */
        if (is_local_field(values[0])) {
          field_item = (Item_field *)(values[0]->real_item());
          if (add_key_equal_fields(thd, key_fields, *and_level, cond_func,
                                   field_item, equal_func, &values[1],
                                   num_values, usable_tables, sargables))
            return true;
        }
        /*
          Append keys for 'value[0] <cmp> field' if the
          condition is of the form:
          'value[0] BETWEEN field1 AND field2'
        */
        for (uint i = 1; i <= num_values; i++) {
          if (is_local_field(values[i])) {
            field_item = (Item_field *)(values[i]->real_item());
            if (add_key_equal_fields(thd, key_fields, *and_level, cond_func,
                                     field_item, equal_func, values, 1,
                                     usable_tables, sargables))
              return true;
          }
        }
      }  // if ( ... Item_func::BETWEEN)
      else if (cond_func->functype() == Item_func::MEMBER_OF_FUNC &&
               is_local_field(cond_func->key_item())) {
        // The predicate is <val> IN (<typed array>)
        add_key_equal_fields(thd, key_fields, *and_level, cond_func,
                             (Item_field *)(cond_func->key_item()->real_item()),
                             true, cond_func->arguments(), 1, usable_tables,
                             sargables);
      } else if (cond_func->functype() == Item_func::JSON_CONTAINS ||
                 cond_func->functype() == Item_func::JSON_OVERLAPS) {
        /*
          Applicability analysis was done during substitute_gc().
          Check here that a typed array field is used and there's a key over
          it.
          1) func has a key item
          2) key item is a local field
          3) key item is a typed array field
          If so, mark appropriate index as available for range optimizer
        */
        if (!cond_func->key_item() ||                  // 1
            !is_local_field(cond_func->key_item()) ||  // 2
            !cond_func->key_item()->returns_array())   // 3
          break;
        const Field *field =
            (down_cast<const Item_field *>(cond_func->key_item()))->field;
        JOIN_TAB *tab = field->table->reginfo.join_tab;
        Key_map possible_keys = field->key_start;

        possible_keys.intersect(field->table->keys_in_use_for_query);
        tab->keys().merge(possible_keys);      // Add possible keys
        tab->const_keys.merge(possible_keys);  // Add possible keys
      }                                        // if (... Item_func::CONTAINS)
      // The predicate is IN or <>
      else if (is_local_field(cond_func->key_item()) &&
               !cond_func->is_outer_reference()) {
        values = cond_func->arguments() + 1;
        if (cond_func->functype() == Item_func::NE_FUNC &&
            is_local_field(cond_func->arguments()[1]))
          values--;
        assert(cond_func->functype() != Item_func::IN_FUNC ||
               cond_func->argument_count() != 2);
        if (add_key_equal_fields(
                thd, key_fields, *and_level, cond_func,
                (Item_field *)(cond_func->key_item()->real_item()), false,
                values, cond_func->argument_count() - 1, usable_tables,
                sargables))
          return true;
      } else if (cond_func->functype() == Item_func::IN_FUNC &&
                 cond_func->key_item()->type() == Item::ROW_ITEM) {
        /*
          The condition is (column1, column2, ... ) IN ((const1_1, const1_2),
          ...) and there is an index on (column1, column2, ...)

          The code below makes sure that the row constructor on the lhs indeed
          contains only column references before calling add_key_field on them.

          We can't do a ref access on IN, yet here we are. Why? We need
          to run add_key_field() only because it verifies that there are
          only constant expressions in the rows on the IN's rhs, see
          comment above the call to add_key_field() below.

          Actually, We could in theory do a ref access if the IN rhs
          contained just a single row, but there is a hack in the parser
          causing such IN predicates be parsed as row equalities.
        */
        Item_row *lhs_row = static_cast<Item_row *>(cond_func->key_item());
        if (is_row_of_local_columns(lhs_row)) {
          for (uint i = 0; i < lhs_row->cols(); ++i) {
            Item *const lhs_item = lhs_row->element_index(i)->real_item();
            assert(lhs_item->type() == Item::FIELD_ITEM);
            Item_field *const lhs_column = static_cast<Item_field *>(lhs_item);
            // j goes from 1 since arguments()[0] is the lhs of IN.
            for (uint j = 1; j < cond_func->argument_count(); ++j) {
              // Here we pick out the i:th column in the j:th row.
              Item *rhs_item = cond_func->arguments()[j];
              assert(rhs_item->type() == Item::ROW_ITEM);
              Item_row *rhs_row = static_cast<Item_row *>(rhs_item);
              assert(rhs_row->cols() == lhs_row->cols());
              Item **rhs_expr_ptr = rhs_row->addr(i);
              /*
                add_key_field() will write a Key_field on each call
                here, but we don't care, it will never be used. We only
                call it for the side effect: update JOIN_TAB::const_keys
                so the range optimizer can be invoked. We pass a
                scrap buffer and pointer here.
              */
              Key_field scrap_key_field = **key_fields;
              Key_field *scrap_key_field_ptr = &scrap_key_field;
              if (add_key_field(thd, &scrap_key_field_ptr, *and_level,
                                cond_func, lhs_column,
                                true,  // eq_func
                                rhs_expr_ptr,
                                1,  // Number of expressions: one
                                usable_tables,
                                nullptr))  // sargables
                return true;
              // The pointer is not supposed to increase by more than one.
              assert(scrap_key_field_ptr <= &scrap_key_field + 1);
            }
          }
        }
      }
      break;
    }
    case Item_func::OPTIMIZE_OP: {
      bool equal_func = (cond_func->functype() == Item_func::EQ_FUNC ||
                         cond_func->functype() == Item_func::EQUAL_FUNC);

      if (is_local_field(cond_func->arguments()[0])) {
        if (add_key_equal_fields(
                thd, key_fields, *and_level, cond_func,
                (Item_field *)(cond_func->arguments()[0])->real_item(),
                equal_func, cond_func->arguments() + 1, 1, usable_tables,
                sargables))
          return true;
      } else {
        Item *real_item = cond_func->arguments()[0]->real_item();
        if (real_item->type() == Item::FUNC_ITEM) {
          Item_func *func_item = down_cast<Item_func *>(real_item);
          if (func_item->functype() == Item_func::COLLATE_FUNC) {
            Item *key_item = func_item->key_item();
            if (key_item->type() == Item::FIELD_ITEM) {
              if (add_key_equal_fields(thd, key_fields, *and_level, cond_func,
                                       down_cast<Item_field *>(key_item),
                                       equal_func, cond_func->arguments() + 1,
                                       1, usable_tables, sargables))
                return true;
            }
          }
        }
      }
      if (is_local_field(cond_func->arguments()[1]) &&
          cond_func->functype() != Item_func::LIKE_FUNC) {
        if (add_key_equal_fields(
                thd, key_fields, *and_level, cond_func,
                (Item_field *)(cond_func->arguments()[1])->real_item(),
                equal_func, cond_func->arguments(), 1, usable_tables,
                sargables))
          return true;
      } else {
        Item *real_item = cond_func->arguments()[1]->real_item();
        if (real_item->type() == Item::FUNC_ITEM) {
          Item_func *func_item = down_cast<Item_func *>(real_item);
          if (func_item->functype() == Item_func::COLLATE_FUNC) {
            Item *key_item = func_item->key_item();
            if (key_item->type() == Item::FIELD_ITEM) {
              if (add_key_equal_fields(thd, key_fields, *and_level, cond_func,
                                       down_cast<Item_field *>(key_item),
                                       equal_func, cond_func->arguments(), 1,
                                       usable_tables, sargables))
                return true;
            }
          }
        }
      }

      break;
    }
    case Item_func::OPTIMIZE_NULL:
      /* column_name IS [NOT] NULL */
      if (is_local_field(cond_func->arguments()[0]) &&
          !cond_func->is_outer_reference()) {
        Item *tmp = new Item_null;
        if (tmp == nullptr) return true;
        if (add_key_equal_fields(
                thd, key_fields, *and_level, cond_func,
                (Item_field *)(cond_func->arguments()[0])->real_item(),
                cond_func->functype() == Item_func::ISNULL_FUNC, &tmp, 1,
                usable_tables, sargables))
          return true;
      }
      break;
    case Item_func::OPTIMIZE_EQUAL:
      Item_equal *item_equal = (Item_equal *)cond;
      Item *const_item = item_equal->get_const();
      if (const_item) {
        /*
          For each field field1 from item_equal consider the equality
          field1=const_item as a condition allowing an index access of the table
          with field1 by the keys value of field1.
        */
        Item_equal_iterator it(*item_equal);
        Item_field *item;
        while ((item = it++)) {
          if (add_key_field(thd, key_fields, *and_level, cond_func, item, true,
                            &const_item, 1, usable_tables, sargables))
            return true;
        }
      } else {
        /*
          Consider all pairs of different fields included into item_equal.
          For each of them (field1, field1) consider the equality
          field1=field2 as a condition allowing an index access of the table
          with field1 by the keys value of field2.
        */
        Item_equal_iterator outer_it(*item_equal);
        Item_equal_iterator inner_it(*item_equal);
        Item_field *outer;
        while ((outer = outer_it++)) {
          Item_field *inner;
          while ((inner = inner_it++)) {
            if (!outer->field->eq(inner->field)) {
              if (add_key_field(thd, key_fields, *and_level, cond_func, outer,
                                true, (Item **)&inner, 1, usable_tables,
                                sargables))
                return true;
            }
          }
          inner_it.rewind();
        }
      }
      break;
  }
  return false;
}

/*
  Add all keys with uses 'field' for some keypart
  If field->and_level != and_level then only mark key_part as const_part

  RETURN
   0 - OK
   1 - Out of memory.
*/

static bool add_key_part(Key_use_array *keyuse_array, Key_field *key_field) {
  if (key_field->eq_func && !(key_field->optimize & KEY_OPTIMIZE_EXISTS)) {
    const Field *const field = key_field->item_field->field;
    TABLE_LIST *const tl = key_field->item_field->table_ref;
    TABLE *const table = tl->table;

    for (uint key = 0; key < table->s->keys; key++) {
      if (!(table->keys_in_use_for_query.is_set(key))) continue;
      if (table->key_info[key].flags & (HA_FULLTEXT | HA_SPATIAL))
        continue;  // ToDo: ft-keys in non-ft queries.   SerG

      uint key_parts = actual_key_parts(&table->key_info[key]);
      for (uint part = 0; part < key_parts; part++) {
        if (field->eq(table->key_info[key].key_part[part].field)) {
          const Key_use keyuse(tl, key_field->val,
                               key_field->val->used_tables(), key, part,
                               key_field->optimize & KEY_OPTIMIZE_REF_OR_NULL,
                               (key_part_map)1 << part,
                               ~(ha_rows)0,  // will be set in optimize_keyuse
                               key_field->null_rejecting, key_field->cond_guard,
                               key_field->sj_pred_no);
          if (keyuse_array->push_back(keyuse))
            return true; /* purecov: inspected */
        }
      }
    }
  }
  return false;
}

/**
   Function parses WHERE condition and add key_use for FT index
   into key_use array if suitable MATCH function is found.
   Condition should be a set of AND expression, OR is not supported.
   MATCH function should be a part of simple expression.
   Simple expression is MATCH only function or MATCH is a part of
   comparison expression ('>=' or '>' operations are supported).
   It also sets FT_HINTS values(op_type, op_value).

   @param keyuse_array      Key_use array
   @param cond              WHERE condition
   @param usable_tables     usable tables
   @param simple_match_expr true if this is the first call false otherwise.
                            if MATCH function is found at first call it means
                            that MATCH is simple expression, otherwise, in case
                            of AND/OR condition this parameter will be false.

   @retval
   true if FT key was added to Key_use array
   @retval
   false if no key was added to Key_use array

*/

static bool add_ft_keys(Key_use_array *keyuse_array, Item *cond,
                        table_map usable_tables, bool simple_match_expr) {
  Item_func_match *cond_func = nullptr;

  if (!cond) return false;

  assert(cond->is_bool_func());

  if (cond->type() == Item::FUNC_ITEM) {
    Item_func *func = down_cast<Item_func *>(cond);
    Item_func::Functype functype = func->functype();
    if (functype == Item_func::MATCH_FUNC) {
      func = down_cast<Item_func *>(func->arguments()[0]);
      functype = func->functype();
    }
    enum ft_operation op_type = FT_OP_NO;
    double op_value = 0.0;
    if (functype == Item_func::FT_FUNC) {
      cond_func = down_cast<Item_func_match *>(func)->get_master();
      cond_func->set_hints_op(op_type, op_value);
    } else if (func->arg_count == 2) {
      Item *arg0 = func->arguments()[0];
      Item *arg1 = func->arguments()[1];
      if (arg1->const_item() && arg0->type() == Item::FUNC_ITEM &&
          down_cast<Item_func *>(arg0)->functype() == Item_func::FT_FUNC &&
          ((functype == Item_func::GE_FUNC &&
            (op_value = arg1->val_real()) > 0) ||
           (functype == Item_func::GT_FUNC &&
            (op_value = arg1->val_real()) >= 0))) {
        cond_func = down_cast<Item_func_match *>(arg0)->get_master();
        if (functype == Item_func::GE_FUNC)
          op_type = FT_OP_GE;
        else if (functype == Item_func::GT_FUNC)
          op_type = FT_OP_GT;
        cond_func->set_hints_op(op_type, op_value);
      } else if (arg0->const_item() && arg1->type() == Item::FUNC_ITEM &&
                 down_cast<Item_func *>(arg1)->functype() ==
                     Item_func::FT_FUNC &&
                 ((functype == Item_func::LE_FUNC &&
                   (op_value = arg0->val_real()) > 0) ||
                  (functype == Item_func::LT_FUNC &&
                   (op_value = arg0->val_real()) >= 0))) {
        cond_func = down_cast<Item_func_match *>(arg1)->get_master();
        if (functype == Item_func::LE_FUNC)
          op_type = FT_OP_GE;
        else if (functype == Item_func::LT_FUNC)
          op_type = FT_OP_GT;
        cond_func->set_hints_op(op_type, op_value);
      }
    }
  } else if (cond->type() == Item::COND_ITEM) {
    List_iterator_fast<Item> li(*down_cast<Item_cond *>(cond)->argument_list());

    if (down_cast<Item_cond *>(cond)->functype() == Item_func::COND_AND_FUNC) {
      Item *item;
      while ((item = li++))
        if (add_ft_keys(keyuse_array, item, usable_tables, false)) return true;
    }
  }

  if (!cond_func || cond_func->key == NO_SUCH_KEY ||
      !(usable_tables & cond_func->table_ref->map()))
    return false;

  TABLE_LIST *tbl = cond_func->table_ref;
  if (!tbl->table->keys_in_use_for_query.is_set(cond_func->key)) return false;

  cond_func->set_simple_expression(simple_match_expr);

  const Key_use keyuse(tbl, cond_func, cond_func->key_item()->used_tables(),
                       cond_func->key, FT_KEYPART,
                       0,            // optimize
                       0,            // keypart_map
                       ~(ha_rows)0,  // ref_table_rows
                       false,        // null_rejecting
                       nullptr,      // cond_guard
                       UINT_MAX);    // sj_pred_no
  tbl->table->reginfo.join_tab->keys().set_bit(cond_func->key);
  return keyuse_array->push_back(keyuse);
}

/**
  Compares two keyuse elements.

  @param a first Key_use element
  @param b second Key_use element

  Compare Key_use elements so that they are sorted as follows:
    -# By table.
    -# By key for each table.
    -# By keypart for each key.
    -# Const values.
    -# Ref_or_null.

  @retval true If a < b.
  @retval false If a >= b.
*/
static bool sort_keyuse(const Key_use &a, const Key_use &b) {
  if (a.table_ref->tableno() != b.table_ref->tableno())
    return a.table_ref->tableno() < b.table_ref->tableno();
  if (a.key != b.key) return a.key < b.key;
  if (a.keypart != b.keypart) return a.keypart < b.keypart;
  // Place const values before other ones
  bool a_const = a.used_tables & ~OUTER_REF_TABLE_BIT;
  bool b_const = b.used_tables & ~OUTER_REF_TABLE_BIT;
  if (a_const != b_const) return b_const;
  /* Place rows that are not 'OPTIMIZE_REF_OR_NULL' first */
  return (a.optimize & KEY_OPTIMIZE_REF_OR_NULL) <
         (b.optimize & KEY_OPTIMIZE_REF_OR_NULL);
}

/*
  Add to Key_field array all 'ref' access candidates within nested join.

    This function populates Key_field array with entries generated from the
    ON condition of the given nested join, and does the same for nested joins
    contained within this nested join.

  @param          thd                 session context
  @param[in]      nested_join_table   Nested join pseudo-table to process
  @param[in,out]  end                 End of the key field array
  @param[in,out]  and_level           And-level
  @param[in,out]  sargables           Array of found sargable candidates

  @returns false if success, true if error

  @note
    We can add accesses to the tables that are direct children of this nested
    join (1), and are not inner tables w.r.t their neighbours (2).

    Example for #1 (outer brackets pair denotes nested join this function is
    invoked for):
    @code
     ... LEFT JOIN (t1 LEFT JOIN (t2 ... ) ) ON cond
    @endcode
    Example for #2:
    @code
     ... LEFT JOIN (t1 LEFT JOIN t2 ) ON cond
    @endcode
    In examples 1-2 for condition cond, we can add 'ref' access candidates to
    t1 only.
    Example #3:
    @code
     ... LEFT JOIN (t1, t2 LEFT JOIN t3 ON inner_cond) ON cond
    @endcode
    Here we can add 'ref' access candidates for t1 and t2, but not for t3.
*/

static bool add_key_fields_for_nj(THD *thd, JOIN *join,
                                  TABLE_LIST *nested_join_table,
                                  Key_field **end, uint *and_level,
                                  SARGABLE_PARAM **sargables) {
  mem_root_deque<TABLE_LIST *> &join_list =
      nested_join_table->nested_join->join_list;
  auto li = join_list.begin();
  auto li_end = join_list.end();
  auto li2 = join_list.begin();
  auto li2_end = join_list.end();
  bool have_another = false;
  table_map tables = 0;
  TABLE_LIST *table;

  while ((table = (li != li_end) ? *li++ : nullptr) ||
         (have_another && li2 != join_list.end() &&
          (li = li2, li_end = li2_end, have_another = false,
           (li != li_end) && (table = *li++)))) {
    if (table->nested_join) {
      if (!table->join_cond_optim()) {
        /* It's a semi-join nest. Walk into it as if it wasn't a nest */
        have_another = true;
        li2 = li;
        li2_end = li_end;
        li = table->nested_join->join_list.begin();
        li_end = table->nested_join->join_list.end();
      } else {
        if (add_key_fields_for_nj(thd, join, table, end, and_level, sargables))
          return true;
      }
    } else if (!table->join_cond_optim())
      tables |= table->map();
  }
  if (nested_join_table->join_cond_optim()) {
    if (add_key_fields(thd, join, end, and_level,
                       nested_join_table->join_cond_optim(), tables, sargables))
      return true;
  }
  return false;
}

///  @} (end of group RefOptimizerModule)


// Source: sql_optimizer.cc
// Lines 4443-7650
