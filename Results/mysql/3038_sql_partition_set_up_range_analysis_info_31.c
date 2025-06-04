static void set_up_range_analysis_info(partition_info *part_info) {
  /* Set the catch-all default */
  part_info->get_part_iter_for_interval = nullptr;
  part_info->get_subpart_iter_for_interval = nullptr;

  /*
    Check if get_part_iter_for_interval_via_mapping() can be used for
    partitioning
  */
  switch (part_info->part_type) {
    case partition_type::RANGE:
    case partition_type::LIST:
      if (!part_info->column_list) {
        if (part_info->part_expr->get_monotonicity_info() != NON_MONOTONIC) {
          part_info->get_part_iter_for_interval =
              get_part_iter_for_interval_via_mapping;
          goto setup_subparts;
        }
      } else {
        part_info->get_part_iter_for_interval =
            get_part_iter_for_interval_cols_via_map;
        goto setup_subparts;
      }
    default:;
  }

  /*
    Check if get_part_iter_for_interval_via_walking() can be used for
    partitioning
  */
  if (part_info->num_part_fields == 1) {
    Field *field = part_info->part_field_array[0];
    switch (field->type()) {
      case MYSQL_TYPE_TINY:
      case MYSQL_TYPE_SHORT:
      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_LONG:
      case MYSQL_TYPE_LONGLONG:
        part_info->get_part_iter_for_interval =
            get_part_iter_for_interval_via_walking;
        break;
      default:;
    }
  }


// Source: sql_partition.cc
// Lines 5362-5405
