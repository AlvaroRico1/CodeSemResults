static void fetch_string_with_conversion(MYSQL_BIND *param, char *value,
                                         size_t length) {
  uchar *buffer = pointer_cast<uchar *>(param->buffer);
  const char *endptr = value + length;

  /*
    This function should support all target buffer types: the rest
    of conversion functions can delegate conversion to it.
  */
  switch (param->buffer_type) {
    case MYSQL_TYPE_NULL: /* do nothing */
      break;
    case MYSQL_TYPE_TINY: {
      int err;
      longlong data = my_strtoll10(value, &endptr, &err);
      *param->error = (IS_TRUNCATED(data, param->is_unsigned, INT_MIN8,
                                    INT_MAX8, UINT_MAX8) ||
                       err > 0);
      *buffer = (uchar)data;
      break;
    }
    case MYSQL_TYPE_SHORT: {
      int err;
      longlong data = my_strtoll10(value, &endptr, &err);
      *param->error = (IS_TRUNCATED(data, param->is_unsigned, INT_MIN16,
                                    INT_MAX16, UINT_MAX16) ||
                       err > 0);
      shortstore(buffer, (short)data);
      break;
    }


// Source: libmysql.cc
// Lines 2695-2724
