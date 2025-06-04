bool check_part_func_fields(Field **ptr, bool ok_with_charsets) {
  Field *field;
  DBUG_TRACE;

  while ((field = *(ptr++))) {
    /*
      For CHAR/VARCHAR fields we need to take special precautions.
      Binary collation with CHAR is automatically supported. Other
      types need some kind of standardisation function handling
    */
    if (field_is_partition_charset(field)) {
      const CHARSET_INFO *cs = field->charset();
      if (!ok_with_charsets || cs->mbmaxlen > 1 || cs->strxfrm_multiply > 1) {
        return true;
      }
    }
  }
  return false;
}


// Source: sql_partition.cc
// Lines 1423-1441
