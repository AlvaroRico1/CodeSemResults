static bool is_binary_compatible(enum enum_field_types type1,
                                 enum enum_field_types type2) {
  static const enum enum_field_types
      range1[] = {MYSQL_TYPE_SHORT, MYSQL_TYPE_YEAR, MYSQL_TYPE_NULL},
      range2[] = {MYSQL_TYPE_INT24, MYSQL_TYPE_LONG, MYSQL_TYPE_NULL},
      range3[] = {MYSQL_TYPE_DATETIME, MYSQL_TYPE_TIMESTAMP, MYSQL_TYPE_NULL},
      range4[] = {
          MYSQL_TYPE_ENUM,        MYSQL_TYPE_SET,       MYSQL_TYPE_TINY_BLOB,
          MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_BLOB,
          MYSQL_TYPE_VAR_STRING,  MYSQL_TYPE_STRING,    MYSQL_TYPE_GEOMETRY,
          MYSQL_TYPE_DECIMAL,     MYSQL_TYPE_NULL};
  static const enum enum_field_types *range_list[] = {range1, range2, range3,
                                                      range4},
                                     **range_list_end =
                                         range_list + sizeof(range_list) /
                                                          sizeof(*range_list);
  const enum enum_field_types **range, *type;

  if (type1 == type2) return true;
  for (range = range_list; range != range_list_end; ++range) {
    /* check that both type1 and type2 are in the same range */
    bool type1_found = false, type2_found = false;
    for (type = *range; *type != MYSQL_TYPE_NULL; type++) {
      type1_found |= type1 == *type;
      type2_found |= type2 == *type;
    }
    if (type1_found || type2_found) return type1_found && type2_found;
  }


// Source: libmysql.cc
// Lines 3386-3413
