static int cmp_rec_and_tuple(part_column_list_val *val, uint32 nvals_in_rec) {
  partition_info *part_info = val->part_info;
  Field **field = part_info->part_field_array;
  Field **fields_end = field + nvals_in_rec;
  int res;

  for (; field != fields_end; field++, val++) {
    if (val->max_value) return -1;
    if ((*field)->is_null()) {
      if (val->null_value) continue;
      return -1;
    }
    if (val->null_value) return +1;
    res = (*field)->cmp(val->column_value.field_image);
    if (res) return res;
  }
  return 0;
}


// Source: sql_partition.cc
// Lines 5480-5497
