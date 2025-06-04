int client_query_attributes::set_params(MYSQL *mysql) {
  if (count == 0) return 0;

  int rc = mysql_bind_param(mysql, count, values, names);
  return rc;
}


// Source: client_query_attributes.cc
// Lines 47-52
