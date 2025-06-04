bool Query_block::nest_derived(THD *thd, Item *join_cond,
                               mem_root_deque<TABLE_LIST *> *nested_join_list,
                               TABLE_LIST *derived_table) {
  // Locate join nest in which the joinee with the condition sits
  const bool found MY_ATTRIBUTE((unused)) = walk_join_list(
      *nested_join_list,
      [join_cond, &nested_join_list](TABLE_LIST *tr) mutable -> bool {
        if (tr->join_cond() == join_cond) {
          nested_join_list = &tr->embedding->nested_join->join_list;
          return true;  // break off walk
        }
        return false;
      });

  assert(found);

  // Make a copy of the join list, outer before inner joinees, so we
  // can rebuild the join_list after inserting the derived table in a nest
  // with the outer(s)
  mem_root_deque<TABLE_LIST *> copy_list(*THR_MALLOC);
  auto &jlist = *nested_join_list;
  for (auto tl : jlist) copy_list.push_front(tl);
  jlist.clear();

  auto it = std::find_if(copy_list.begin(), copy_list.end(),
                         [join_cond](TABLE_LIST *tl) -> bool {
                           return tl->join_cond() == join_cond;
                         });
  assert(it != copy_list.end());  // assert that we found it
  const size_t idx = it - copy_list.begin();

  // Insert back all outer tables to the inner containing the condition.
  // Normally only one.
  for (size_t i = 0; i < idx; i++) {
    jlist.push_front(copy_list[i]);
  }

  // Insert the derived table and nest it with the outer(s)
  jlist.push_front(derived_table);
  derived_table->join_list = &jlist;
  derived_table->embedding = copy_list[idx]->embedding;

  if (nest_join(thd, this, copy_list[idx]->embedding, &jlist, idx + 1,
                "(nest_join)") == nullptr)
    return true;

  // Insert back the inner containing the JOIN condition and any subsequent
  // joinees
  for (size_t i = idx; i < copy_list.size(); i++) {
    jlist.push_front(copy_list[i]);
  }

  return false;
}


// Source: sql_resolver.cc
// Lines 6448-6501
