static uint32 get_next_partition_id_list(PARTITION_ITERATOR *part_iter) {
  if (part_iter->part_nums.cur >= part_iter->part_nums.end) {
    if (part_iter->ret_null_part) {
      part_iter->ret_null_part = false;
      return part_iter->part_info->has_null_part_id;
    }
    part_iter->part_nums.cur = part_iter->part_nums.start;
    part_iter->ret_null_part = part_iter->ret_null_part_orig;
    return NOT_A_PARTITION_ID;
  } else {
    partition_info *part_info = part_iter->part_info;
    uint32 num_part = part_iter->part_nums.cur++;
    if (part_info->column_list) {
      uint num_columns = part_info->part_field_list.elements;
      return part_info->list_col_array[num_part * num_columns].partition_id;
    }
    return part_info->list_array[num_part].partition_id;
  }


// Source: sql_partition.cc
// Lines 6051-6068
