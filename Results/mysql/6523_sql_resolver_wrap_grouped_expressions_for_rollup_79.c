static ReplaceResult wrap_grouped_expressions_for_rollup(
    Query_block *select, Item *item, Item *parent, unsigned argument_idx) {
  if (is_rollup_group_wrapper(item->real_item())) {
    // This item must already be a group item, or we wouldn't have
    // wrapped it earlier. No need to do anything more about it,
    // since it's already wrapped (also, don't traverse further).
    return {ReplaceResult::REPLACE, item};
  }

  int rollup_level = 0;
  ORDER *group = select->find_in_group_list(item, &rollup_level);
  if (group != nullptr) {
    Item_rollup_group_item *new_item =
        new Item_rollup_group_item(rollup_level, item);
    if (new_item == nullptr || select->rollup_group_items.push_back(new_item)) {
      return {ReplaceResult::ERROR, nullptr};
    }
    new_item->quick_fix_field();
    if (group->rollup_item == nullptr) {
      group->rollup_item = new_item;
    }
    return {ReplaceResult::REPLACE, new_item};
  } else if (parent != nullptr && parent->type() == Item::FUNC_ITEM &&


// Source: sql_resolver.cc
// Lines 4651-4673
