    SEMIJOIN = 1 << 3
  };
  uint changes = 0;          // To keep track of changes.
  if (changelog == nullptr)  // This is the top call.
    changelog = &changes;

  NESTED_JOIN *nested_join;
  TABLE_LIST *prev_table = nullptr;
  const bool straight_join = active_options() & SELECT_STRAIGHT_JOIN;
  DBUG_TRACE;

  /*
    Try to simplify join operations from join_list.
    The most outer join operation is checked for conversion first.
    join_list is a join nest, and 'cond' is a condition which acts as a filter
    applied to the nest's operation (post-filter).
    Thus, considering this example:
    (A LEFT JOIN B ON JC) WHERE W ,
    we'll "confront W with A LEFT JOIN B": this will, recursively,
    - confront W with B,
    - confront W with A.
    Because W is external to the nest, if W would be false when B is
    NULL-complemented we know we can change LEFT JOIN to JOIN.
    We will not confront JC with B or A, it wouldn't make sense, as JC isn't a
    post-filter for their join operation.
    Another example:
    (A LEFT JOIN (B LEFT JOIN C ON JC2) ON JC1) WHERE W ,
    while confronting W with (B LEFT JOIN C), we will also, as first step,
    confront JC1 with (B LEFT JOIN C), and thus recursively confront JC1
    with C and then with B.
    Another example:
    (A LEFT JOIN (B SEMI JOIN C ON JC2) ON JC1) WHERE W ,
    while confronting W with (B SEMI JOIN C), if W is known false we will

  */
  for (TABLE_LIST *table : *join_list) {
    table_map used_tables;
    table_map not_null_tables = (table_map)0;

    if ((nested_join = table->nested_join)) {
      /*
         If the element of join_list is a nested join apply
         the procedure to its nested join list first.
         This confronts the join nest's condition with each member of the
         nest.
      */
      if (table->join_cond()) {
        Item *join_cond = table->join_cond();
        /*
           If a join condition JC is attached to the table,
           check all null rejected predicates in this condition.
           If such a predicate over an attribute belonging to
           an inner table of an embedded outer join is found,
           the outer join is converted to an inner join and
           the corresponding join condition is added to JC.
        */
        if (simplify_joins(
                thd, &nested_join->join_list,
                false,  // not 'top' as it's not WHERE.
                // SJ nests can dissolve into upper SJ or anti SJ nests:
                in_sj || table->is_sj_or_aj_nest(), &join_cond, changelog))
          return true;

        if (join_cond != table->join_cond()) {
          assert(join_cond);
          table->set_join_cond(join_cond);
        }
      }
      nested_join->used_tables = (table_map)0;
      nested_join->not_null_tables = (table_map)0;
      // This recursively confronts "cond" with each member of the nest
      if (simplify_joins(thd, &nested_join->join_list,
                         top,  // if it was WHERE it still is
                         in_sj || table->is_sj_or_aj_nest(), cond, changelog))
        return true;
      used_tables = nested_join->used_tables;
      not_null_tables = nested_join->not_null_tables;
    } else {
      used_tables = table->map();
      if (*cond) not_null_tables = (*cond)->not_null_tables();
    }

    if (table->embedding) {
      table->embedding->nested_join->used_tables |= used_tables;
      table->embedding->nested_join->not_null_tables |= not_null_tables;
    }

    if (!table->outer_join || (used_tables & not_null_tables)) {
      /*
        For some of the inner tables there are conjunctive predicates
        that reject nulls => the outer join can be replaced by an inner join.
      */
      if (table->outer_join) {
        *changelog |= OUTER_JOIN_TO_INNER;
        table->outer_join = false;
      }
      if (table->join_cond()) {
        *changelog |= JOIN_COND_TO_WHERE;
        /* Add join condition to the WHERE or upper-level join condition. */
        if (*cond) {
          Item *i1 = *cond, *i2 = table->join_cond();
          /*
            User supplied stored procedures in the query can violate row-level
            filter enforced by a view. So make sure view's filter conditions
            precede any other conditions.
          */
          if (table->is_view() && i1->has_stored_program()) {
            std::swap(i1, i2);
          }

          Item_cond_and *new_cond =
              down_cast<Item_cond_and *>(and_conds(i1, i2));
          if (!new_cond) return true;
          new_cond->apply_is_true();
          /*
            It is always a new item as both the upper-level condition and a
            join condition existed
          */
          assert(!new_cond->fixed);
          Item *cond_after_fix = new_cond;
          if (new_cond->fix_fields(thd, &cond_after_fix)) return true;

          if (new_cond == cond_after_fix) {
          }
          *cond = cond_after_fix;
        } else {
          *cond = table->join_cond();
        }


// Source: sql_resolver.cc
// Lines 1877-2004
