bool is_indexed_agg_distinct(JOIN *join,
                             mem_root_deque<Item_field *> *out_args) {
  Item_sum **sum_item_ptr;
  bool result = false;
  Field_map first_aggdistinct_fields;

  if (join->primary_tables > 1 ||             /* reference more than 1 table */
      join->select_distinct ||                /* or a DISTINCT */
      join->query_block->olap == ROLLUP_TYPE) /* Check (B3) for ROLLUP */
    return false;

  if (join->make_sum_func_list(*join->fields, true)) return false;

  for (sum_item_ptr = join->sum_funcs; *sum_item_ptr; sum_item_ptr++) {
    Item_sum *sum_item = *sum_item_ptr;
    Field_map cur_aggdistinct_fields;
    Item *expr;
    /* aggregate is not AGGFN(DISTINCT) or more than 1 argument to it */
    switch (sum_item->sum_func()) {
      case Item_sum::MIN_FUNC:
      case Item_sum::MAX_FUNC:
        continue;
      case Item_sum::COUNT_DISTINCT_FUNC:
        break;
      case Item_sum::AVG_DISTINCT_FUNC:
      case Item_sum::SUM_DISTINCT_FUNC:
        if (sum_item->argument_count() == 1) break;
      /* fall through */
      default:
        return false;
    }

    for (uint i = 0; i < sum_item->argument_count(); i++) {
      expr = sum_item->get_arg(i);
      /* The AGGFN(DISTINCT) arg is not an attribute? */
      if (expr->real_item()->type() != Item::FIELD_ITEM) return false;

      Item_field *item = static_cast<Item_field *>(expr->real_item());
      if (out_args) out_args->push_back(item);

      cur_aggdistinct_fields.set_bit(item->field->field_index());
      result = true;
    }
    /*
      If there are multiple aggregate functions, make sure that they all
      refer to exactly the same set of columns.
    */
    if (first_aggdistinct_fields.is_clear_all())
      first_aggdistinct_fields.merge(cur_aggdistinct_fields);
    else if (first_aggdistinct_fields != cur_aggdistinct_fields)
      return false;
  }


// Source: sql_optimizer.cc
// Lines 7684-7735
