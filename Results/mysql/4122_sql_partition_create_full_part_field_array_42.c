static bool create_full_part_field_array(THD *thd, TABLE *table,
                                         partition_info *part_info) {
  bool result = false;
  Field **ptr;
  my_bitmap_map *bitmap_buf;
  DBUG_TRACE;

  if (!part_info->is_sub_partitioned()) {
    part_info->full_part_field_array = part_info->part_field_array;
    part_info->num_full_part_fields = part_info->num_part_fields;
  } else {
    Field *field, **field_array;
    uint num_part_fields = 0, size_field_array;
    ptr = table->field;
    while ((field = *(ptr++))) {
      if (field->is_flag_set(FIELD_IN_PART_FUNC_FLAG)) num_part_fields++;
    }
    size_field_array = (num_part_fields + 1) * sizeof(Field *);
    field_array = (Field **)sql_calloc(size_field_array);
    if (unlikely(!field_array)) {
      mem_alloc_error(size_field_array);
      result = true;
      goto end;
    }
    num_part_fields = 0;
    ptr = table->field;
    while ((field = *(ptr++))) {
      if (field->is_flag_set(FIELD_IN_PART_FUNC_FLAG))
        field_array[num_part_fields++] = field;
    }
    field_array[num_part_fields] = nullptr;
    part_info->full_part_field_array = field_array;
    part_info->num_full_part_fields = num_part_fields;
  }


// Source: sql_partition.cc
// Lines 547-580
