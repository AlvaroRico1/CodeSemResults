void QEP_TAB::push_index_cond(const JOIN_TAB *join_tab, uint keyno,
                              Opt_trace_object *trace_obj) {
  JOIN *const join_ = join();
  DBUG_TRACE;

  ASSERT_BEST_REF_IN_JOIN_ORDER(join_);
  assert(join_tab == join_->best_ref[idx()]);

  if (join_tab->reversed_access)  // @todo: historical limitation, lift it!
    return;

  TABLE *const tbl = table();

  // Disable ICP for Innodb intrinsic temp table because of performance
  if (tbl->s->db_type() == innodb_hton && tbl->s->tmp_table != NO_TMP_TABLE &&
      tbl->s->tmp_table != TRANSACTIONAL_TMP_TABLE)
    return;

  // TODO: Currently, index on virtual generated column doesn't support ICP
  if (tbl->vfield && tbl->index_contains_some_virtual_gcol(keyno)) return;

  /*
    Fields of other non-const tables aren't allowed in following cases:
       type is:
        (JT_ALL | JT_INDEX_SCAN | JT_RANGE | JT_INDEX_MERGE)
       and BNL is used.
    and allowed otherwise.
  */
  const bool other_tbls_ok =
      !((type() == JT_ALL || type() == JT_INDEX_SCAN || type() == JT_RANGE ||
         type() == JT_INDEX_MERGE) &&
        join_tab->use_join_cache() == JOIN_CACHE::ALG_BNL);

  /*
    We will only attempt to push down an index condition when the
    following criteria are true:
    0. The table has a select condition
    1. The storage engine supports ICP.
    2. The index_condition_pushdown switch is on and
       the use of ICP is not disabled by the NO_ICP hint.
    3. The query is not a multi-table update or delete statement. The reason
       for this requirement is that the same handler will be used
       both for doing the select/join and the update. The pushed index
       condition might then also be applied by the storage engine
       when doing the update part and result in either not finding
       the record to update or updating the wrong record.
    4. The JOIN_TAB is not part of a subquery that has guarded conditions
       that can be turned on or off during execution of a 'Full scan on NULL
       key'.
       @see Item_in_optimizer::val_int()
       @see subselect_iterator_engine::exec()
       @see TABLE_REF::cond_guards
       @see setup_join_buffering
    5. The join type is not CONST or SYSTEM. The reason for excluding
       these join types, is that these are optimized to only read the
       record once from the storage engine and later re-use it. In a
       join where a pushed index condition evaluates fields from
       tables earlier in the join sequence, the pushed condition would
       only be evaluated the first time the record value was needed.
    6. The index is not a clustered index. The performance improvement
       of pushing an index condition on a clustered key is much lower
       than on a non-clustered key. This restriction should be
       re-evaluated when WL#6061 is implemented.
    7. The index on virtual generated columns is not supported for ICP.
  */
  if (condition() &&
      tbl->file->index_flags(keyno, 0, true) & HA_DO_INDEX_COND_PUSHDOWN &&
      hint_key_state(join_->thd, table_ref, keyno, ICP_HINT_ENUM,
                     OPTIMIZER_SWITCH_INDEX_CONDITION_PUSHDOWN) &&
      join_->thd->lex->sql_command != SQLCOM_UPDATE_MULTI &&
      join_->thd->lex->sql_command != SQLCOM_DELETE_MULTI &&
      !has_guarded_conds() && type() != JT_CONST && type() != JT_SYSTEM &&
      !(keyno == tbl->s->primary_key &&
        tbl->file->primary_key_is_clustered())) {
    DBUG_EXECUTE("where", print_where(join_->thd, condition(), "full cond",
                                      QT_ORDINARY););
    Item *idx_cond =
        make_cond_for_index(condition(), tbl, keyno, other_tbls_ok);
    DBUG_EXECUTE("where",
                 print_where(join_->thd, idx_cond, "idx cond", QT_ORDINARY););
    if (idx_cond) {
      /*
        Check that the condition to push actually contains fields from
        the index. Without any fields from the index it is unlikely
        that it will filter out any records since the conditions on
        fields from other tables in most cases have already been
        evaluated.
      */
      idx_cond->update_used_tables();
      if ((idx_cond->used_tables() & table_ref->map()) == 0) {
        /*
          The following assert is to check that we only skip pushing the
          index condition for the following situations:
          1. We actually are allowed to generate an index condition on another
             table.
          2. The index condition is a constant item.
          3. The index condition contains an updatable user variable
             (test this by checking that the RAND_TABLE_BIT is set).
        */
        assert(other_tbls_ok ||                    // 1
               idx_cond->const_item() ||           // 2
               idx_cond->is_non_deterministic());  // 3
        return;
      }

      Item *idx_remainder_cond = nullptr;

      /*
        For BKA cache we store condition to special BKA cache field
        because evaluation of the condition requires additional operations
        before the evaluation. This condition is used in
        JOIN_CACHE_BKA::skip_index_tuple() functions.
      */
      if (join_tab->use_join_cache() &&
          /*
            if cache is used then the value is true only
            for BKA cache (see setup_join_buffering() func).
            In this case other_tbls_ok is an equivalent of
            cache->is_key_access().
          */
          other_tbls_ok &&
          (idx_cond->used_tables() &
           ~(table_ref->map() | join_->const_table_map))) {
        cache_idx_cond = idx_cond;
        trace_obj->add("pushed_to_BKA", true);
      } else {
        idx_remainder_cond = tbl->file->idx_cond_push(keyno, idx_cond);
        DBUG_EXECUTE("where",
                     print_where(join_->thd, tbl->file->pushed_idx_cond,
                                 "icp cond", QT_ORDINARY););
      }
      /*
        Disable eq_ref's "lookup cache" if we've pushed down an index
        condition.
        TODO: This check happens to work on current ICP implementations, but
        there may exist a compliant implementation that will not work
        correctly with it. Sort this out when we stabilize the condition
        pushdown APIs.
      */
      if (idx_remainder_cond != idx_cond) {
        ref().disable_cache = true;
        trace_obj->add("pushed_index_condition", idx_cond);
      }

      Item *row_cond = make_cond_remainder(condition(), true);
      DBUG_EXECUTE("where", print_where(join_->thd, row_cond, "remainder cond",
                                        QT_ORDINARY););

      if (row_cond) {
        and_conditions(&row_cond, idx_remainder_cond);
        idx_remainder_cond = row_cond;
      }
      set_condition(idx_remainder_cond);
      trace_obj->add("table_condition_attached", idx_remainder_cond);
    }


// Source: sql_select.cc
// Lines 2758-2912
