void Query_block::update_used_tables() {
  for (Item *item : visible_fields()) {
    item->update_used_tables();
  }
  if (join_list != nullptr) update_used_tables_for_join(join_list);
  if (where_cond() != nullptr) where_cond()->update_used_tables();
  for (ORDER *group = group_list.first; group; group = group->next)
    (*group->item)->update_used_tables();
  if (having_cond() != nullptr) having_cond()->update_used_tables();
  for (ORDER *order = order_list.first; order; order = order->next)
    (*order->item)->update_used_tables();
  List_iterator<Window> wi(m_windows);
  Window *w;
  while ((w = wi++)) {
    for (ORDER *wp = w->first_partition_by(); wp != nullptr; wp = wp->next)
      (*wp->item)->update_used_tables();
    for (ORDER *wo = w->first_order_by(); wo != nullptr; wo = wo->next)
      (*wo->item)->update_used_tables();
  }
}


// Source: sql_resolver.cc
// Lines 821-840
