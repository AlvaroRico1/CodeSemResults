static void copy_bind_parameter_values(THD *thd, PS_PARAM *parameters,
                                       unsigned long count) {
  thd->bind_parameter_values = parameters;
  thd->bind_parameter_values_count = count;
  unsigned long inx;
  PS_PARAM *par;
  for (inx = 0, par = thd->bind_parameter_values; inx < count; inx++, par++) {
    if (par->name_length && par->name) {
      void *newd = thd->alloc(par->name_length);
      memcpy(newd, par->name, par->name_length);
      par->name = reinterpret_cast<unsigned char *>(newd);
    }
    if (par->length && par->value) {
      void *newd = thd->alloc(par->length);
      memcpy(newd, par->value, par->length);
      par->value = reinterpret_cast<unsigned char *>(newd);
    }


// Source: sql_parse.cc
// Lines 1482-1498
