bool is_prefix_index(TABLE *table, uint idx) {
  if (!table || !table->key_info) {
    return false;
  }

  KEY *key_info = table->key_info;
  uint key_parts = key_info[idx].user_defined_key_parts;
  KEY_PART_INFO *key_part = key_info[idx].key_part;

  for (uint i = 0; i < key_parts; i++, key_part++) {
    if (key_part->field &&
        !(table->field[key_part->fieldnr - 1]
              ->part_of_prefixkey.is_clear_all()) &&
        !(key_info->flags & (HA_FULLTEXT | HA_SPATIAL))) {
      return true;
    }
  }
  return false;
}


// Source: sql_optimizer.cc
// Lines 1556-1574
