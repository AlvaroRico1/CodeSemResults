static void restore_part_field_pointers(Field **ptr, uchar **restore_ptr) {
  Field *field;
  while ((field = *(ptr++))) {
    field->set_field_ptr(*restore_ptr);
    restore_ptr++;
  }
  return;
}


// Source: sql_partition.cc
// Lines 2708-2715
