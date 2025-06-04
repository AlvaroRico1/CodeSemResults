static void clear_field_flag(TABLE *table) {
  Field **ptr;
  DBUG_TRACE;

  for (ptr = table->field; *ptr; ptr++)
    (*ptr)->clear_flag(GET_FIXED_FIELDS_FLAG);
}


// Source: sql_partition.cc
// Lines 704-710
