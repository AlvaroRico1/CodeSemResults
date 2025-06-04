static void set_indicator_in_key_fields(KEY *key_info) {
  KEY_PART_INFO *key_part;
  uint key_parts = key_info->user_defined_key_parts, i;
  for (i = 0, key_part = key_info->key_part; i < key_parts; i++, key_part++)
    key_part->field->set_flag(GET_FIXED_FIELDS_FLAG);
}


// Source: sql_partition.cc
// Lines 652-657
