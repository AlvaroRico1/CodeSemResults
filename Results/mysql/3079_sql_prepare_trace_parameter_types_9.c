void Prepared_statement::trace_parameter_types() {
  if (param_count == 0) return;
  Opt_trace_object anon(&thd->opt_trace);
  Opt_trace_array typ(&thd->opt_trace, "statement_parameters");
  Item_param **end = param_array + param_count;
  char buf[50];

  for (Item_param **it = param_array; it < end; ++it) {
    enum_field_types t = (*it)->data_type();
    const char *n = fieldtype2str(t);
    switch (t) {
      case MYSQL_TYPE_NEWDECIMAL:
      case MYSQL_TYPE_TIME:
      case MYSQL_TYPE_DATETIME:
        snprintf(buf, sizeof(buf), "%s decimals=%d", n, (*it)->decimals);
        n = buf;
      default:;
    }
    typ.add_alnum(n);
  }


// Source: sql_prepare.cc
// Lines 1471-1490
