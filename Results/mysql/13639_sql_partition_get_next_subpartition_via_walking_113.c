static uint32 get_next_subpartition_via_walking(PARTITION_ITERATOR *part_iter) {
  Field *field = part_iter->part_info->subpart_field_array[0];
  uint32 res;
  if (part_iter->field_vals.cur == part_iter->field_vals.end) {
    part_iter->field_vals.cur = part_iter->field_vals.start;
    return NOT_A_PARTITION_ID;
  }
  field->store(part_iter->field_vals.cur++, field->is_flag_set(UNSIGNED_FLAG));
  if (part_iter->part_info->get_subpartition_id(part_iter->part_info, &res))
    return NOT_A_PARTITION_ID;
  return res;
}


// Source: sql_partition.cc
// Lines 6109-6120
