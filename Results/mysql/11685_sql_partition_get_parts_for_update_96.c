int get_parts_for_update(const uchar *old_data,
                         const uchar *new_data MY_ATTRIBUTE((unused)),
                         const uchar *rec0, partition_info *part_info,
                         uint32 *old_part_id, uint32 *new_part_id,
                         longlong *new_func_value) {
  Field **part_field_array = part_info->full_part_field_array;
  int error;
  longlong old_func_value;
  DBUG_TRACE;

  assert(new_data == rec0);  // table->record[0]
  set_field_ptr(part_field_array, old_data, rec0);
  error = part_info->get_partition_id(part_info, old_part_id, &old_func_value);
  set_field_ptr(part_field_array, rec0, old_data);
  if (unlikely(error)) {
    part_info->err_value = old_func_value;
    return error;
  }
  if (unlikely((error = part_info->get_partition_id(part_info, new_part_id,
                                                    new_func_value)))) {
    part_info->err_value = *new_func_value;
    return error;
  }
  return 0;
}


// Source: sql_partition.cc
// Lines 314-338
