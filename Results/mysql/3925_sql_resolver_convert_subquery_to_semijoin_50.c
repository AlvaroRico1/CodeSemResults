bool Query_block::convert_subquery_to_semijoin(
    THD *thd, Item_exists_subselect *subq_pred) {
  TABLE_LIST *emb_tbl_nest = nullptr;
  mem_root_deque<TABLE_LIST *> *emb_join_list = &top_join_list;
  DBUG_TRACE;

  assert(subq_pred->substype() == Item_subselect::IN_SUBS ||
         subq_pred->substype() == Item_subselect::EXISTS_SUBS);

  Opt_trace_context *trace = &thd->opt_trace;
  Opt_trace_object trace_object(trace, "transformation_to_semi_join");
  if (unlikely(trace->is_started())) {
    trace_object.add("subquery_predicate", subq_pred);
  }

  bool outer_join = false;  // True if predicate is inner to an outer join

  // Save the set of tables in the outer query block:
  table_map outer_tables_map = all_tables_map();
  const bool do_aj = subq_pred->can_do_aj;

  /*
    Find out where to insert the semi-join nest and the generated condition.

    For t1 LEFT JOIN t2, embedding_join_nest will be t2.
    Note that t2 may be a simple table or may itself be a join nest
    (e.g. in the case t1 LEFT JOIN (t2 JOIN t3))
  */
  if (subq_pred->embedding_join_nest != nullptr) {
    // Is this on inner side of an outer join?
    outer_join = subq_pred->embedding_join_nest->is_inner_table_of_outer_join();

    if (subq_pred->embedding_join_nest->nested_join) {
      /*
        We're dealing with

          ... [LEFT] JOIN  ( ... ) ON (subquery AND condition) ...

        The sj-nest will be inserted into the brackets nest.
      */
      emb_tbl_nest = subq_pred->embedding_join_nest;
      emb_join_list = &emb_tbl_nest->nested_join->join_list;
    } else if (!subq_pred->embedding_join_nest->outer_join) {
      /*
        We're dealing with

          ... INNER JOIN tblX ON (subquery AND condition) ...

        The sj-nest will be tblX's "sibling", i.e. another child of its
        parent. This is ok because tblX is joined as an inner join.
      */
      emb_tbl_nest = subq_pred->embedding_join_nest->embedding;
      if (emb_tbl_nest) emb_join_list = &emb_tbl_nest->nested_join->join_list;
    } else {
      TABLE_LIST *outer_tbl = subq_pred->embedding_join_nest;
      /*
        We're dealing with

          ... LEFT JOIN tbl ON (on_expr AND subq_pred) ...

        tbl will be replaced with:

          ( tbl SJ (subq_tables) )
          |                      |
          |<----- wrap_nest ---->|

        giving:
          ... LEFT JOIN ( tbl SJ (subq_tables) ) ON (on_expr AND subq_pred) ...

        Q:  other subqueries may be pointing to this element. What to do?
        A1: simple solution: copy *subq_pred->embedding_join_nest= *parent_nest.
            But we'll need to fix other pointers.
        A2: Another way: have TABLE_LIST::next_ptr so the following
            subqueries know the table has been nested.
        A3: changes in the TABLE_LIST::outer_join will make everything work
            automatically.
      */
      TABLE_LIST *const wrap_nest = TABLE_LIST::new_nested_join(
          thd->mem_root, "(sj-wrap)", outer_tbl->embedding,
          outer_tbl->join_list, this);
      if (wrap_nest == nullptr) return true;

      wrap_nest->nested_join->join_list.push_back(outer_tbl);

      outer_tbl->embedding = wrap_nest;
      outer_tbl->join_list = &wrap_nest->nested_join->join_list;

      /*
        wrap_nest will take place of outer_tbl, so move the outer join flag
        and join condition.
      */
      wrap_nest->outer_join = outer_tbl->outer_join;
      outer_tbl->outer_join = false;

      wrap_nest->set_join_cond(outer_tbl->join_cond());
      outer_tbl->set_join_cond(nullptr);

      for (auto li = wrap_nest->join_list->begin();
           li != wrap_nest->join_list->end(); ++li) {
        TABLE_LIST *tbl = *li;
        if (tbl == outer_tbl) {
          *li = wrap_nest;
          break;
        }
      }

      /*
        outer_tbl is replaced by wrap_nest. Any subquery which was attached to
        outer_tbl must be attached to embedding_join_nest instead.
      */
      for (Item_exists_subselect *subquery : (*sj_candidates)) {
        if (subquery->embedding_join_nest == outer_tbl)
          subquery->embedding_join_nest = wrap_nest;
      }

      /*
        Ok now wrap_nest 'contains' outer_tbl and we're ready to add the
        semi-join nest into it
      */
      emb_join_list = &wrap_nest->nested_join->join_list;
      emb_tbl_nest = wrap_nest;
    }
  }
  // else subquery is in WHERE.

  if (do_aj) {
    /*
      A negated IN/EXISTS like:
      NOT EXISTS(... FROM subq_tables WHERE subq_cond)
      The above code has ensured that we have one of these 3 situations:

      (a) FROM ... WHERE (subquery AND condition)
      (emb_tbl_nest == nullptr, emb_join_list == FROM clause)

      which has to be changed to
          FROM (...)            LEFT JOIN (subq_tables) ON subq_cond
               ^ aj-left-nest             ^aj-nest
          WHERE x IS NULL AND condition

      or:
      (b) ... [LEFT] JOIN ( ...          ) ON (subquery AND condition) ...
                          ^ emb_tbl_nest, emb_join_list

      which has to be changed to
          ... [LEFT] JOIN ( (...)          LEFT JOIN (subq_tables) ON subq_cond)
                            ^aj-left-nest            ^aj-nest
                          ^ emb_tbl_nest, emb_join_list
              ON x IS NULL AND condition ...

      or:
      (c) ... INNER JOIN tblX ON (subquery AND condition) ...
          ^ emb_tbl_nest, emb_join_list
            (if no '()' above this INNER JOIN up to the root, emb_tbl_nest ==
             nullptr and emb_join_list == FROM clause)

      which has to be changed to
       ( ... INNER JOIN tblX ON condition) LEFT JOIN (subq_tables) ON subq_cond
       ^aj-left-nest                                 ^aj-nest

      so:
      - move all tables of emb_join_list into a new aj-left-nest
      - emb_join_list is now empty
      - put subq_tables in a new aj-nest
      - add the subq's subq_cond to aj-nest's ON
      - add a LEFT JOIN operator between the aj-left-nest and aj-nest, with
      ON condition subq_cond.
      - insert aj-nest and aj-left-nest into emb_join_list
      - for some reason, a LEFT JOIN must always be wrapped into a nest (call
      nest_last_join() then)
      - do not yet add 'x IS NULL to WHERE' (add it in optimization phase when
      we have the QEP_TABs so we can set up the 'found'/'not_null_compl'
      pointers in trig conds).
    */
    TABLE_LIST *const wrap_nest = TABLE_LIST::new_nested_join(
        thd->mem_root, "(aj-left-nest)", emb_tbl_nest, emb_join_list, this);
    if (wrap_nest == nullptr) return true;

    // Go through tables of emb_join_list, insert them in wrap_nest
    for (TABLE_LIST *outer_tbl : *emb_join_list) {
      wrap_nest->nested_join->join_list.push_back(outer_tbl);
      outer_tbl->embedding = wrap_nest;
      outer_tbl->join_list = &wrap_nest->nested_join->join_list;
    }
    // FROM clause is now only the new left nest
    emb_join_list->clear();
    emb_join_list->push_back(wrap_nest);
    outer_join = true;
  }


// Source: sql_resolver.cc
// Lines 2912-3099
