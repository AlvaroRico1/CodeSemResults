static int get_partition_id_range_col(partition_info *part_info,
                                      uint32 *part_id, longlong *) {
  part_column_list_val *range_col_array = part_info->range_col_array;
  uint num_columns = part_info->part_field_list.elements;
  uint max_partition = part_info->num_parts - 1;
  uint min_part_id = 0;
  uint max_part_id = max_partition;
  uint loc_part_id;
  DBUG_TRACE;

  while (max_part_id > min_part_id) {
    loc_part_id = (max_part_id + min_part_id + 1) >> 1;
    if (cmp_rec_and_tuple(range_col_array + loc_part_id * num_columns,
                          num_columns) >= 0)
      min_part_id = loc_part_id + 1;
    else
      max_part_id = loc_part_id - 1;
  }
  loc_part_id = max_part_id;
  if (loc_part_id != max_partition)
    if (cmp_rec_and_tuple(range_col_array + loc_part_id * num_columns,
                          num_columns) >= 0)
      loc_part_id++;
  *part_id = (uint32)loc_part_id;
  if (loc_part_id == max_partition &&
      (cmp_rec_and_tuple(range_col_array + loc_part_id * num_columns,
                         num_columns) >= 0))
    return HA_ERR_NO_PARTITION_FOUND;

  DBUG_PRINT("exit", ("partition: %d", *part_id));
  return 0;
}


// Source: sql_partition.cc
// Lines 3032-3063
