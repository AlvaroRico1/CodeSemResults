static int get_sub_part_id_from_key(const TABLE *table, uchar *buf,
                                    KEY *key_info, const key_range *key_spec,
                                    uint32 *part_id) {
  uchar *rec0 = table->record[0];
  partition_info *part_info = table->part_info;
  int res;
  DBUG_TRACE;

  key_restore(buf, key_spec->key, key_info, key_spec->length);
  if (likely(rec0 == buf)) {
    res = part_info->get_subpartition_id(part_info, part_id);
  } else {
    Field **part_field_array = part_info->subpart_field_array;
    set_field_ptr(part_field_array, buf, rec0);
    res = part_info->get_subpartition_id(part_info, part_id);
    set_field_ptr(part_field_array, rec0, buf);
  }
  return res;
}


// Source: sql_partition.cc
// Lines 3422-3440
