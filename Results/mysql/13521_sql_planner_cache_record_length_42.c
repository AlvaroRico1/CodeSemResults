static uint cache_record_length(JOIN *join, uint idx) {
  uint length = 0;
  JOIN_TAB **pos, **end;

  for (pos = join->best_ref + join->const_tables, end = join->best_ref + idx;
       pos != end; pos++) {
    JOIN_TAB *join_tab = *pos;
    if (!join_tab->used_fieldlength)  // Not calculated yet
    {
      /*
        (1) needs_rowid: we don't know if Duplicate Weedout may be
        used, length will thus be inaccurate, this is acceptable.
      */
      calc_used_field_length(join_tab->table(),
                             false,  // (1)
                             &join_tab->used_fieldlength);
    }
    length += join_tab->used_fieldlength;
  }


// Source: sql_planner.cc
// Lines 93-111
