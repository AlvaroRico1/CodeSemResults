void JOIN::update_sargable_from_const(SARGABLE_PARAM *sargables) {
  for (; sargables->field; sargables++) {
    Field *const field = sargables->field;
    JOIN_TAB *const tab = field->table->reginfo.join_tab;
    Key_map possible_keys = field->key_start;
    possible_keys.intersect(field->table->keys_in_use_for_query);
    bool is_const = true;
    for (uint j = 0; j < sargables->num_values; j++)
      is_const &= sargables->arg_value[j]->const_item();
    if (is_const) {
      tab->const_keys.merge(possible_keys);
      tab->keys().merge(possible_keys);
    }
  }


// Source: sql_optimizer.cc
// Lines 5544-5557
