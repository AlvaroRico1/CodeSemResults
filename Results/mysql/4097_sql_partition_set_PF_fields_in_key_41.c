static bool set_PF_fields_in_key(KEY *key_info, uint key_length) {
  KEY_PART_INFO *key_part;
  bool found_part_field = false;
  DBUG_TRACE;

  for (key_part = key_info->key_part; (int)key_length > 0; key_part++) {
    if (key_part->null_bit) key_length--;
    if (key_part->type == HA_KEYTYPE_BIT) {
      if (((Field_bit *)key_part->field)->bit_len) key_length--;
    }
    if (key_part->key_part_flag & (HA_BLOB_PART + HA_VAR_LENGTH_PART)) {
      key_length -= HA_KEY_BLOB_LENGTH;
    }
    if (key_length < key_part->length) break;
    key_length -= key_part->length;
    if (key_part->field->is_flag_set(FIELD_IN_PART_FUNC_FLAG)) {
      found_part_field = true;
      key_part->field->set_flag(GET_FIXED_FIELDS_FLAG);
    }
  }
  return found_part_field;
}


// Source: sql_partition.cc
// Lines 3352-3373
