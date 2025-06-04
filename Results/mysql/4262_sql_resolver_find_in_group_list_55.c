ORDER *Query_block::find_in_group_list(Item *item, int *rollup_level) const {
  Item *real_item = item->real_item();
  ORDER *best_candidate = nullptr;
  int idx = 0;
  for (ORDER *group = group_list.first; group; group = group->next, ++idx) {
    Item *group_item = *group->item;
    if (real_item->eq(group_item->real_item(), /*binary_cmp=*/false)) {
      if (item->item_name.ptr() != nullptr &&
          group_item->item_name.ptr() != nullptr &&
          item->item_name.eq(group_item->item_name)) {
        // Match on group _and_ alias; return immediately.
        if (rollup_level != nullptr) {
          *rollup_level = idx;
        }
        return group;
      } else if (best_candidate == nullptr) {
        // Match on group but not alias; it's a good candidate,
        // but only if we don't find a better match. (If there
        // are multiple such candidates, we use the leftmost one.)
        if (rollup_level != nullptr) {
          *rollup_level = idx;
        }
        best_candidate = group;
      }
    }
  }


// Source: sql_resolver.cc
// Lines 4609-4634
