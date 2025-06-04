Item *Query_block::single_visible_field() const {
  Item *ret = nullptr;
  for (Item *item : visible_fields()) {
    if (ret != nullptr) {
      // More than one.
      return nullptr;
    }
    ret = item;
  }
  return ret;
}


// Source: sql_resolver.cc
// Lines 4748-4758
