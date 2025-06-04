int get_part_for_delete(const uchar *buf, const uchar *rec0,
                        partition_info *part_info, uint32 *part_id) {
  int error;
  longlong func_value;
  DBUG_TRACE;

  if (likely(buf == rec0)) {
    if (unlikely((error = part_info->get_partition_id(part_info, part_id,
                                                      &func_value)))) {
      part_info->err_value = func_value;
      return error;
    }
    DBUG_PRINT("info", ("Delete from partition %d", *part_id));
  } else {
    Field **part_field_array = part_info->full_part_field_array;
    set_field_ptr(part_field_array, buf, rec0);
    error = part_info->get_partition_id(part_info, part_id, &func_value);
    set_field_ptr(part_field_array, rec0, buf);
    if (unlikely(error)) {
      part_info->err_value = func_value;
      return error;
    }
    DBUG_PRINT("info", ("Delete from partition %d (path2)", *part_id));
  }


// Source: sql_partition.cc
// Lines 361-384
