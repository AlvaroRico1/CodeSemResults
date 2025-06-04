static uint32 get_next_partition_via_walking(PARTITION_ITERATOR *part_iter) {
  uint32 part_id;
  Field *field = part_iter->part_info->part_field_array[0];
  while (part_iter->field_vals.cur != part_iter->field_vals.end) {
    longlong dummy;
    field->store(part_iter->field_vals.cur++,
                 field->is_flag_set(UNSIGNED_FLAG));
    if ((part_iter->part_info->is_sub_partitioned() &&
         !part_iter->part_info->get_part_partition_id(part_iter->part_info,
                                                      &part_id, &dummy)) ||
        !part_iter->part_info->get_partition_id(part_iter->part_info, &part_id,
                                                &dummy))
      return part_id;
  }
  part_iter->field_vals.cur = part_iter->field_vals.start;
  return NOT_A_PARTITION_ID;
}


// Source: sql_partition.cc
// Lines 6089-6105
