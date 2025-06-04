static void fetch_datetime_with_conversion(MYSQL_BIND *param,
                                           MYSQL_FIELD *field,
                                           MYSQL_TIME *my_time) {
  switch (param->buffer_type) {
    case MYSQL_TYPE_NULL: /* do nothing */
      break;
    case MYSQL_TYPE_DATE:
      *(MYSQL_TIME *)(param->buffer) = *my_time;
      *param->error = my_time->time_type != MYSQL_TIMESTAMP_DATE;
      break;
    case MYSQL_TYPE_TIME:
      *(MYSQL_TIME *)(param->buffer) = *my_time;
      *param->error = my_time->time_type != MYSQL_TIMESTAMP_TIME;
      break;
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
      *(MYSQL_TIME *)(param->buffer) = *my_time;
      /* No error: time and date are compatible with datetime */
      break;
    case MYSQL_TYPE_YEAR:
      shortstore(pointer_cast<uchar *>(param->buffer), my_time->year);
      *param->error = true;
      break;
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE: {
      ulonglong value = TIME_to_ulonglong(*my_time);
      fetch_float_with_conversion(param, field, ulonglong2double(value),
                                  MY_GCVT_ARG_DOUBLE);
      break;
    }
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG: {
      longlong value = (longlong)TIME_to_ulonglong(*my_time);
      fetch_long_with_conversion(param, field, value, true);
      break;
    }
    default: {
      /*
        Convert time value  to string and delegate the rest to
        fetch_string_with_conversion:
      */
      char buff[MAX_DATE_STRING_REP_LENGTH];
      uint length = my_TIME_to_str(*my_time, buff, field->decimals);
      /* Resort to string conversion */
      fetch_string_with_conversion(param, (char *)buff, length);
      break;
    }


// Source: libmysql.cc
// Lines 3069-3118
