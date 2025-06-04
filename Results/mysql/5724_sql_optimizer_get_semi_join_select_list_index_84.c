static uint get_semi_join_select_list_index(Item_field *item_field) {
  TABLE_LIST *emb_sj_nest = item_field->table_ref->embedding;
  if (emb_sj_nest && emb_sj_nest->is_sj_or_aj_nest()) {
    const mem_root_deque<Item *> &items =
        emb_sj_nest->nested_join->sj_inner_exprs;
    for (size_t i = 0; i < items.size(); i++) {
      const Item *sel_item = items[i];
      if (sel_item->type() == Item::FIELD_ITEM &&
          down_cast<const Item_field *>(sel_item)->field->eq(item_field->field))
        return i;
    }
  }
  return UINT_MAX;
}


// Source: sql_optimizer.cc
// Lines 6640-6653
