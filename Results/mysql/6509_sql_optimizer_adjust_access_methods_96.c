void JOIN::adjust_access_methods() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  for (uint i = const_tables; i < tables; i++) {
    JOIN_TAB *const tab = best_ref[i];
    TABLE_LIST *const tl = tab->table_ref;

    if (tab->type() == JT_ALL) {
      /*
       It's possible to speedup query by switching from full table scan to
       the scan of covering index, due to less data being read.
       Prerequisites for this are:
       1) Keyread (i.e index only scan) is allowed (table isn't updated/deleted
         from)
       2) Covering indexes are available
       3) This isn't a derived table/materialized view
      */
      if (!tab->table()->no_keyread &&                    //  1
          !tab->table()->covering_keys.is_clear_all() &&  //  2
          !tl->uses_materialization())                    //  3
      {
        /*
        It has turned out that the change commented out below, while speeding
        things up for disk-bound loads, slows them down for cases when the data
        is in disk cache (see BUG#35850):
        //  See bug #26447: "Using the clustered index for a table scan
        //  is always faster than using a secondary index".
        if (table->s->primary_key != MAX_KEY &&
            table->file->primary_key_is_clustered())
          tab->index= table->s->primary_key;
        else
          tab->index=find_shortest_key(table, & table->covering_keys);
        */
        if (tab->position()->sj_strategy != SJ_OPT_LOOSE_SCAN)
          tab->set_index(
              find_shortest_key(tab->table(), &tab->table()->covering_keys));
        tab->set_type(JT_INDEX_SCAN);  // Read with index_first / index_next
        // From table scan to index scan, thus filter effect needs no recalc.
      }
    } else if (tab->type() == JT_REF) {
      if (can_switch_from_ref_to_range(thd, tab, ORDER_NOT_RELEVANT, false)) {
        tab->set_type(JT_RANGE);

        Opt_trace_context *const trace = &thd->opt_trace;
        Opt_trace_object wrapper(trace);
        Opt_trace_object(trace, "access_type_changed")
            .add_utf8_table(tl)
            .add_utf8("index",
                      tab->table()->key_info[tab->position()->key->key].name)
            .add_alnum("old_type", "ref")
            .add_alnum("new_type", join_type_str[tab->type()])
            .add_alnum("cause", "uses_more_keyparts");

        tab->use_quick = QS_RANGE;
        tab->position()->filter_effect = COND_FILTER_STALE;
      } else {
        // Cleanup quick, REF/REF_OR_NULL/EQ_REF, will be clarified later
        delete tab->quick();
        tab->set_quick(nullptr);
      }
    }
    // Ensure AM consistency
    assert(!(tab->quick() && (tab->type() == JT_REF || tab->type() == JT_ALL)));
    assert((tab->type() != JT_RANGE && tab->type() != JT_INDEX_MERGE) ||
           tab->quick());
    if (!tab->const_keys.is_clear_all() &&
        tab->table()->reginfo.impossible_range &&
        ((i == const_tables && tab->type() == JT_REF) ||
         ((tab->type() == JT_ALL || tab->type() == JT_RANGE ||
           tab->type() == JT_INDEX_MERGE || tab->type() == JT_INDEX_SCAN) &&
          tab->use_quick != QS_RANGE)) &&
        !tab->table_ref->is_inner_table_of_outer_join())
      zero_result_cause = "Impossible WHERE noticed after reading const tables";
  }


// Source: sql_optimizer.cc
// Lines 2697-2769
