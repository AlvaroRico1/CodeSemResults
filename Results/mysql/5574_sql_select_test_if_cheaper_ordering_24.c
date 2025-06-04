bool test_if_cheaper_ordering(const JOIN_TAB *tab, ORDER_with_src *order,
                              TABLE *table, Key_map usable_keys, int ref_key,
                              ha_rows select_limit, int *new_key,
                              int *new_key_direction, ha_rows *new_select_limit,
                              uint *new_used_key_parts,
                              uint *saved_best_key_parts) {
  DBUG_TRACE;
  /*
    Check whether there is an index compatible with the given order
    usage of which is cheaper than usage of the ref_key index (ref_key>=0)
    or a table scan.
    It may be the case if ORDER/GROUP BY is used with LIMIT.
  */
  ha_rows best_select_limit = HA_POS_ERROR;
  JOIN *join = tab ? tab->join() : nullptr;
  if (join) ASSERT_BEST_REF_IN_JOIN_ORDER(join);
  uint nr;
  uint best_key_parts = 0;
  int best_key_direction = 0;
  ha_rows best_records = 0;
  double read_time;
  int best_key = -1;
  bool is_best_covering = false;
  double fanout = 1;
  ha_rows table_records = table->file->stats.records;
  bool group = join && join->grouped && order == &join->group_list;
  double refkey_rows_estimate =
      static_cast<double>(table->quick_condition_rows);
  const bool has_limit = (select_limit != HA_POS_ERROR);
  const join_type cur_access_method = tab ? tab->type() : JT_ALL;

  if (join) {
    read_time = tab->position()->read_cost;
    for (uint jt = tab->idx() + 1; jt < join->primary_tables; jt++) {
      POSITION *pos = join->best_ref[jt]->position();
      fanout *= pos->rows_fetched * pos->filter_effect;
      if (fanout < 0) break;  // fanout became 'unknown'
    }
  } else
    read_time = table->file->table_scan_cost().total_cost();

  /*
    Calculate the selectivity of the ref_key for REF_ACCESS. For
    RANGE_ACCESS we use table->quick_condition_rows.
  */
  if (ref_key >= 0 && cur_access_method == JT_REF) {
    if (table->quick_keys.is_set(ref_key))
      refkey_rows_estimate = static_cast<double>(table->quick_rows[ref_key]);
    else {
      const KEY *ref_keyinfo = table->key_info + ref_key;
      if (ref_keyinfo->has_records_per_key(tab->ref().key_parts - 1))
        refkey_rows_estimate =
            ref_keyinfo->records_per_key(tab->ref().key_parts - 1);
      else
        refkey_rows_estimate = 1.0;  // No index statistics
    }
    assert(refkey_rows_estimate >= 1.0);
  }

  for (nr = 0; nr < table->s->keys; nr++) {
    int direction = 0;
    uint used_key_parts;
    bool skip_quick;

    if (usable_keys.is_set(nr) &&
        (direction = test_if_order_by_key(order, table, nr, &used_key_parts,
                                          &skip_quick))) {
      bool is_covering = table->covering_keys.is_set(nr) ||
                         (nr == table->s->primary_key &&
                          table->file->primary_key_is_clustered());
      // Don't allow backward scans on indexes with mixed ASC/DESC key parts
      if (skip_quick) table->quick_keys.clear_bit(nr);

      /*
        Don't use an index scan with ORDER BY without limit.
        For GROUP BY without limit always use index scan
        if there is a suitable index.
        Why we hold to this asymmetry hardly can be explained
        rationally. It's easy to demonstrate that using
        temporary table + filesort could be cheaper for grouping
        queries too.
      */
      if (is_covering || select_limit != HA_POS_ERROR ||
          (ref_key < 0 && (group || table->force_index_order))) {
        rec_per_key_t rec_per_key;
        KEY *keyinfo = table->key_info + nr;
        if (select_limit == HA_POS_ERROR) select_limit = table_records;
        if (group) {
          /*
            Used_key_parts can be larger than keyinfo->key_parts
            when using a secondary index clustered with a primary
            key (e.g. as in Innodb).
            See Bug #28591 for details.
          */
          rec_per_key =
              used_key_parts && used_key_parts <= actual_key_parts(keyinfo)
                  ? keyinfo->records_per_key(used_key_parts - 1)
                  : 1.0f;
          rec_per_key = std::max(rec_per_key, 1.0f);
          /*
            With a grouping query each group containing on average
            rec_per_key records produces only one row that will
            be included into the result set.
          */
          if (select_limit > table_records / rec_per_key)
            select_limit = table_records;
          else
            select_limit = (ha_rows)(select_limit * rec_per_key);
        }
        /*
          If tab=tk is not the last joined table tn then to get first
          L records from the result set we can expect to retrieve
          only L/fanout(tk,tn) where fanout(tk,tn) says how many
          rows in the record set on average will match each row tk.
          Usually our estimates for fanouts are too pessimistic.
          So the estimate for L/fanout(tk,tn) will be too optimistic
          and as result we'll choose an index scan when using ref/range
          access + filesort will be cheaper.
        */
        if (fanout == 0)                // Would have been a division-by-zero
          select_limit = HA_POS_ERROR;  // -> 'infinite'
        else if (fanout > 0)            // 'fanout' not unknown
          select_limit =
              (ha_rows)(select_limit < fanout ? 1 : select_limit / fanout);
        /*
          We assume that each of the tested indexes is not correlated
          with ref_key. Thus, to select first N records we have to scan
          N/selectivity(ref_key) index entries.
          selectivity(ref_key) = #scanned_records/#table_records =
          refkey_rows_estimate/table_records.
          In any case we can't select more than #table_records.
          N/(refkey_rows_estimate/table_records) > table_records
          <=> N > refkey_rows_estimate.
          Neither should it be possible to select more than #table_records
          rows from refkey_rows_estimate.
         */
        if (select_limit > refkey_rows_estimate)
          select_limit = table_records;
        else if (table_records >= refkey_rows_estimate)
          select_limit = (ha_rows)(select_limit * (double)table_records /
                                   refkey_rows_estimate);
        rec_per_key =
            keyinfo->records_per_key(keyinfo->user_defined_key_parts - 1);
        rec_per_key = std::max(rec_per_key, 1.0f);
        /*
          Here we take into account the fact that rows are
          accessed in sequences rec_per_key records in each.
          Rows in such a sequence are supposed to be ordered
          by rowid/primary key. When reading the data
          in a sequence we'll touch not more pages than the
          table file contains.
          TODO. Use the formula for a disk sweep sequential access
          to calculate the cost of accessing data rows for one
          index entry.
        */
        const Cost_estimate table_scan_time = table->file->table_scan_cost();
        const double index_scan_time =
            select_limit / rec_per_key *
            min<double>(table->cost_model()->page_read_cost(rec_per_key),
                        table_scan_time.total_cost());

        /*
          Switch to index that gives order if its scan time is smaller than
          read_time of current chosen access method. In addition, if the
          current chosen access method is index scan or table scan, always
          switch to the index that gives order when it is covering or when
          force index order or group by is present.
        */
        if (((cur_access_method == JT_ALL ||
              cur_access_method == JT_INDEX_SCAN) &&
             (is_covering || group || table->force_index_order)) ||
            index_scan_time < read_time) {
          ha_rows quick_records = table_records;
          const ha_rows refkey_select_limit =
              (ref_key >= 0 && table->covering_keys.is_set(ref_key))
                  ? static_cast<ha_rows>(refkey_rows_estimate)
                  : HA_POS_ERROR;

          if ((is_best_covering && !is_covering) ||
              (is_covering && refkey_select_limit < select_limit))
            continue;
          if (table->quick_keys.is_set(nr))
            quick_records = table->quick_rows[nr];
          if (best_key < 0 ||
              (select_limit <= min(quick_records, best_records)
                   ? keyinfo->user_defined_key_parts < best_key_parts
                   : quick_records < best_records) ||
              // We assume forward scan is faster than backward even if the
              // key is longer. This should be taken into account in cost
              // calculation.
              direction > best_key_direction) {
            best_key = nr;
            best_key_parts = keyinfo->user_defined_key_parts;
            if (saved_best_key_parts) *saved_best_key_parts = used_key_parts;
            best_records = quick_records;
            is_best_covering = is_covering;
            best_key_direction = direction;
            best_select_limit = select_limit;
          }
        }
      }


// Source: sql_select.cc
// Lines 4946-5146
