bool partition_key_modified(TABLE *table, const MY_BITMAP *fields) {
  Field **fld;
  partition_info *part_info = table->part_info;
  DBUG_TRACE;

  if (!part_info) return false;
  if (table->s->db_type()->partition_flags &&
      (table->s->db_type()->partition_flags() & HA_CAN_UPDATE_PARTITION_KEY))
    return false;
  for (fld = part_info->full_part_field_array; *fld; fld++)
    if (bitmap_is_set(fields, (*fld)->field_index())) return true;
  return false;
}


// Source: sql_partition.cc
// Lines 2472-2484
