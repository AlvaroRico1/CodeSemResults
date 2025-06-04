static bool find_field_in_order_list(Field *field, void *data) {
  ORDER *group = (ORDER *)data;
  bool part_found = false;
  for (ORDER *tmp_group = group; tmp_group; tmp_group = tmp_group->next) {
    const Item *item = (*tmp_group->item)->real_item();
    if (item->type() == Item::FIELD_ITEM &&
        down_cast<const Item_field *>(item)->field->eq(field)) {
      part_found = true;
      break;
    }
  }
  return part_found;
}


// Source: sql_optimizer.cc
// Lines 10194-10206
