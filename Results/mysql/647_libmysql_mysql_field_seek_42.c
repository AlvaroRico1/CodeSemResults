MYSQL_FIELD_OFFSET STDCALL mysql_field_seek(MYSQL_RES *result,
                                            MYSQL_FIELD_OFFSET field_offset) {
  MYSQL_FIELD_OFFSET return_value = result->current_field;
  result->current_field = field_offset;
  return return_value;
}


// Source: libmysql.cc
// Lines 699-704
