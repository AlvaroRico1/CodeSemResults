static bool find_field_in_item_list(Field *field, void *data) {
  mem_root_deque<Item *> *fields =
      reinterpret_cast<mem_root_deque<Item *> *>(data);
  bool part_found = false;

  for (const Item *item : VisibleFields(*fields)) {
    if (item->type() == Item::FIELD_ITEM &&
        down_cast<const Item_field *>(item)->field->eq(field)) {
      part_found = true;
      break;
    }
  }
  return part_found;
}


// Source: sql_optimizer.cc
// Lines 10222-10235
