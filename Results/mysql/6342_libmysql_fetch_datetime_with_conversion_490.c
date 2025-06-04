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


// Source: libmysql.cc
// Lines 3069-3098
