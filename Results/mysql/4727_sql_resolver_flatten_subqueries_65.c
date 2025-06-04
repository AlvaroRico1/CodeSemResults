bool Query_block::flatten_subqueries(THD *thd) {
  DBUG_TRACE;

  assert(has_sj_candidates());

  Item_exists_subselect **subq, **subq_begin = sj_candidates->begin(),
                                **subq_end = sj_candidates->end();

  Opt_trace_context *const trace = &thd->opt_trace;

  /*
    Semijoin flattening is bottom-up. Indeed, we have this execution flow,
    for SELECT#1 WHERE X IN (SELECT #2 WHERE Y IN (SELECT#3)) :

    Query_block::prepare() (select#1)
       -> fix_fields() on IN condition
           -> Query_block::prepare() on subquery (select#2)
               -> fix_fields() on IN condition
                    -> Query_block::prepare() on subquery (select#3)
                    <- Query_block::prepare()
               <- fix_fields()
               -> flatten_subqueries: merge #3 in #2
               <- flatten_subqueries
           <- Query_block::prepare()
       <- fix_fields()
       -> flatten_subqueries: merge #2 in #1

    Note that flattening of #(N) is done by its parent JOIN#(N-1), because
    there are cases where flattening is not possible and only the parent can
    know.
   */
  uint subq_no;
  for (subq = subq_begin, subq_no = 0; subq < subq_end; subq++, subq_no++) {
    auto subq_item = *subq;
    /*
      Some subqueries may have been deleted, remove them fully before sorting
      sj_candidates and subsequent processing:
    */
    if (subq_item->strategy == Subquery_strategy::DELETED) {
      sj_candidates->erase_value(subq_item);
      subq--;  // So that the next iteration will handle the next subquery.
      subq_end = sj_candidates->end();  // array's end moved.

      continue;
    }
    // Transformation of IN and EXISTS subqueries is supported
    assert(subq_item->substype() == Item_subselect::IN_SUBS ||
           subq_item->substype() == Item_subselect::EXISTS_SUBS);

    Query_block *child_query_block = subq_item->unit->first_query_block();

    // Check that we proceeded bottom-up
    assert(child_query_block->sj_candidates == nullptr);

    bool dependent = subq_item->unit->uncacheable & UNCACHEABLE_DEPENDENT;
    subq_item->sj_convert_priority =
        (((dependent * MAX_TABLES_FOR_SIZE) +  // dependent subqueries first
          child_query_block->leaf_table_count) *
         65536) +           // then with many tables
        (65536 - subq_no);  // then based on position

    /*
      We may actually allocate more than 64k subqueries in a query block,
      but this is so unlikely that we ignore the impact it may have on sorting.
     */
  }

  /*
    Pick which subqueries to convert:
      sort the subquery array
      - prefer correlated subqueries over uncorrelated;
      - prefer subqueries that have greater number of outer tables;
  */
  std::sort(subq_begin, subq_begin + sj_candidates->size(),
            [](Item_exists_subselect *el1, Item_exists_subselect *el2) {
              return el1->sj_convert_priority > el2->sj_convert_priority;
            });

  // A permanent transformation is going to start, so:
  Prepared_stmt_arena_holder ps_arena_holder(thd);

  // Transform certain subquery predicates to derived tables
  for (subq = subq_begin; subq < subq_end; subq++) {
    auto subq_item = *subq;
    if (subq_item->strategy != Subquery_strategy::CANDIDATE_FOR_DERIVED_TABLE)
      continue;
    OPT_TRACE_TRANSFORM(trace, oto0, oto1,
                        subq_item->unit->first_query_block()->select_number,
                        "IN (SELECT)", "joined derived table");
    oto1.add("chosen", true);
    if (transform_table_subquery_to_join_with_derived(thd, subq_item))
      return true;
  }
  /*
    Replace all subqueries to be flattened with a truth predicate.
    Generally, this predicate is TRUE, but if the subquery has a WHERE condition
    that is always false, replace with a FALSE predicate. In the latter case,
    also avoid converting the subquery to a semi-join.
  */

  uint table_count = leaf_table_count;
  for (subq = subq_begin; subq < subq_end; subq++) {
    auto subq_item = *subq;
    if (subq_item->strategy != Subquery_strategy::CANDIDATE_FOR_SEMIJOIN)
      continue;

    // Add the tables in the subquery nest plus one in case of materialization:
    const uint tables_added =
        subq_item->unit->first_query_block()->leaf_table_count + 1;

    // (1) Not too many tables in total.
    // (2) This subquery contains no antijoin nest (anti/semijoin nest cannot
    // include antijoin nest for implementation reasons, see
    // advance_sj_state()).
    if (table_count + tables_added <= MAX_TABLES &&           // (1)
        !subq_item->unit->first_query_block()->has_aj_nests)  // (2)
      subq_item->strategy = Subquery_strategy::SEMIJOIN;

    Item *subq_where = subq_item->unit->first_query_block()->where_cond();
    /*
      A predicate can be evaluated to ALWAYS TRUE or ALWAYS FALSE when it
      has only const items. If found to be ALWAYS FALSE, do not include
      the subquery in transformations.
    */
    bool cond_value = true;
    if (subq_where && subq_where->const_item() &&
        !subq_where->walk(&Item::is_non_const_over_literals, enum_walk::POSTFIX,
                          nullptr) &&
        simplify_const_condition(thd, &subq_where, false, &cond_value))
      return true;

    if (!cond_value) {
      // Unlink and delete this subquery's query expression
      Item::Cleanup_after_removal_context ctx(this);
      subq_item->walk(&Item::clean_up_after_removal,
                      enum_walk::SUBQUERY_POSTFIX, pointer_cast<uchar *>(&ctx));
    }

    if (subq_item->strategy == Subquery_strategy::SEMIJOIN)
      table_count += tables_added;

    if (subq_item->strategy != Subquery_strategy::SEMIJOIN &&
        subq_item->strategy != Subquery_strategy::DELETED) {
      subq_item->strategy = Subquery_strategy::UNSPECIFIED;
      continue;
    }
    /*
      In WHERE/ON of parent query, replace IN (subq) with truth value:
      - When subquery is converted to anti/semi-join: truth value true.
      - When subquery WHERE cond is false: IN returns FALSE, so truth value
      false if a semijoin (IN) and truth value true if an antijoin (NOT IN).
    */
    Item *truth_item =
        (cond_value || subq_item->can_do_aj)
            ? implicit_cast<Item *>(new (thd->mem_root) Item_func_true())
            : implicit_cast<Item *>(new (thd->mem_root) Item_func_false());
    if (truth_item == nullptr) return true;
    Item **tree = (subq_item->embedding_join_nest == nullptr)
                      ? &m_where_cond
                      : subq_item->embedding_join_nest->join_cond_ref();
    if (replace_subcondition(thd, tree, subq_item, truth_item, false))
      return true; /* purecov: inspected */
  }

  /* Transform the selected subqueries into semi-join */

  for (subq = subq_begin; subq < subq_end; subq++) {
    auto subq_item = *subq;
    if (subq_item->strategy != Subquery_strategy::SEMIJOIN) continue;

    OPT_TRACE_TRANSFORM(
        trace, oto0, oto1, subq_item->unit->first_query_block()->select_number,
        "IN (SELECT)", subq_item->can_do_aj ? "antijoin" : "semijoin");
    oto1.add("chosen", true);
    if (convert_subquery_to_semijoin(thd, *subq)) return true;
  }
  /*
    Finalize the subqueries that we did not convert,
    ie. perform IN->EXISTS rewrite.
  */
  for (subq = subq_begin; subq < subq_end; subq++) {
    auto subq_item = *subq;
    if (subq_item->strategy != Subquery_strategy::UNSPECIFIED) continue;
    Item_subselect::trans_res res;
    subq_item->changed = false;
    subq_item->fixed = false;

    Query_block *save_query_block = thd->lex->current_query_block();
    thd->lex->set_current_query_block(subq_item->unit->first_query_block());

    // This is the only part of the function which uses a JOIN.
    res = subq_item->select_transformer(thd,
                                        subq_item->unit->first_query_block());

    thd->lex->set_current_query_block(save_query_block);

    if (res == Item_subselect::RES_ERROR) return true;

    subq_item->changed = true;
    subq_item->fixed = true;

    /*
      If the Item has been substituted with another Item (e.g an
      Item_in_optimizer), resolve it and add it to proper WHERE or ON clause.
      If no substitute exists (e.g for EXISTS predicate), no action is required.
    */
    Item *substitute = subq_item->substitution;
    if (substitute == nullptr) continue;
    const bool do_fix_fields = !substitute->fixed;
    const bool subquery_in_join_clause =
        subq_item->embedding_join_nest != nullptr;

    Item **tree = subquery_in_join_clause
                      ? (subq_item->embedding_join_nest->join_cond_ref())
                      : &m_where_cond;
    if (replace_subcondition(thd, tree, *subq, substitute, do_fix_fields))
      return true;
    subq_item->substitution = nullptr;
  }

  sj_candidates->clear();
  return false;
}


// Source: sql_resolver.cc
// Lines 3719-3941
