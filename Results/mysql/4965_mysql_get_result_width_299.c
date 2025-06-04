static int get_result_width(MYSQL_RES *result) {
  unsigned int len = 0;
  MYSQL_FIELD *field;
  MYSQL_FIELD_OFFSET offset;

#ifndef NDEBUG
  offset = mysql_field_tell(result);
  assert(offset == 0);
#else
  offset = 0;
#endif

  while ((field = mysql_fetch_field(result)) != nullptr)
    len +=
        get_field_disp_length(field) + 3; /* plus bar, space, & final space */

  (void)mysql_field_seek(result, offset);

  return len + 1; /* plus final bar. */
}


// Source: mysql.cc
// Lines 3723-3742
