static bool get_part_id_from_key(const TABLE *table, uchar *buf, KEY *key_info,
                                 const key_range *key_spec, uint32 *part_id) {
  bool result;
  uchar *rec0 = table->record[0];
  partition_info *part_info = table->part_info;
  longlong func_value;
  DBUG_TRACE;

  key_restore(buf, key_spec->key, key_info, key_spec->length);
  if (likely(rec0 == buf)) {
    result = part_info->get_part_partition_id(part_info, part_id, &func_value);
  } else {
    Field **part_field_array = part_info->part_field_array;
    set_field_ptr(part_field_array, buf, rec0);
    result = part_info->get_part_partition_id(part_info, part_id, &func_value);
    set_field_ptr(part_field_array, rec0, buf);
  }


// Source: sql_partition.cc
// Lines 3463-3479
