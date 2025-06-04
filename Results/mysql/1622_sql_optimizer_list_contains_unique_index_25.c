static bool list_contains_unique_index(JOIN_TAB *tab,
                                       bool (*find_func)(Field *, void *),
                                       void *data) {
  TABLE *table = tab->table();

  if (tab->is_inner_table_of_outer_join()) return false;
  for (uint keynr = 0; keynr < table->s->keys; keynr++) {
    if (keynr == table->s->primary_key ||
        (table->key_info[keynr].flags & HA_NOSAME)) {
      KEY *keyinfo = table->key_info + keynr;
      KEY_PART_INFO *key_part, *key_part_end;

      for (key_part = keyinfo->key_part,
          key_part_end = key_part + keyinfo->user_defined_key_parts;
           key_part < key_part_end; key_part++) {
        if (key_part->field->is_nullable() || !find_func(key_part->field, data))
          break;
      }
      if (key_part == key_part_end) return true;
    }


// Source: sql_optimizer.cc
// Lines 10156-10175
