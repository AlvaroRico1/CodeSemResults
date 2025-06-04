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
  return false;
}


// Source: libmysql.cc
// Lines 3398-3415
