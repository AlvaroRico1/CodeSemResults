void get_full_part_id_from_key(const TABLE *table, uchar *buf, KEY *key_info,
                               const key_range *key_spec,
                               part_id_range *part_spec) {
  bool result;
  partition_info *part_info = table->part_info;
  uchar *rec0 = table->record[0];
  longlong func_value;
  DBUG_TRACE;

  key_restore(buf, key_spec->key, key_info, key_spec->length);
  if (likely(rec0 == buf)) {
    result = part_info->get_partition_id(part_info, &part_spec->start_part,
                                         &func_value);
  } else {
    Field **part_field_array = part_info->full_part_field_array;
    set_field_ptr(part_field_array, buf, rec0);
    result = part_info->get_partition_id(part_info, &part_spec->start_part,
                                         &func_value);
    set_field_ptr(part_field_array, rec0, buf);
  }
  part_spec->end_part = part_spec->start_part;
  if (unlikely(result)) part_spec->start_part++;
}


// Source: sql_partition.cc
// Lines 3504-3526
