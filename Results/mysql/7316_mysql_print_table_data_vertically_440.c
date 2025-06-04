static void print_table_data_vertically(MYSQL_RES *result) {
  MYSQL_ROW cur;
  uint max_length = 0;
  MYSQL_FIELD *field;

  while ((field = mysql_fetch_field(result))) {
    uint length = field->name_length;
    if (length > max_length) max_length = length;
    field->max_length = length;
  }


// Source: mysql.cc
// Lines 3834-3843
