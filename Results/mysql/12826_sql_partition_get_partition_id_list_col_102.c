static int get_partition_id_list_col(partition_info *part_info, uint32 *part_id,
                                     longlong *) {
  part_column_list_val *list_col_array = part_info->list_col_array;
  uint num_columns = part_info->part_field_list.elements;
  int list_index, cmp;
  int min_list_index = 0;
  int max_list_index = part_info->num_list_values - 1;
  DBUG_TRACE;

  while (max_list_index >= min_list_index) {
    list_index = (max_list_index + min_list_index) >> 1;
    cmp = cmp_rec_and_tuple(list_col_array + list_index * num_columns,
                            num_columns);
    if (cmp > 0)
      min_list_index = list_index + 1;
    else if (cmp < 0) {
      if (!list_index) goto notfound;
      max_list_index = list_index - 1;
    } else {
      *part_id = (uint32)list_col_array[list_index * num_columns].partition_id;
      return 0;
    }
  }
notfound:
  *part_id = 0;
  return HA_ERR_NO_PARTITION_FOUND;
}


// Source: sql_partition.cc
// Lines 2819-2845
