bool client_query_attributes::push_param(char *name, char *value) {
  if (count >= max_count) return true;
  names[count] = my_strdup(PSI_NOT_INSTRUMENTED, name, MYF(0));
  memset(&values[count], 0, sizeof(MYSQL_BIND));
  unsigned val_len = strlen(value);
  values[count].buffer = my_malloc(PSI_NOT_INSTRUMENTED, val_len + 1, MYF(0));
  if (val_len) memcpy(values[count].buffer, value, val_len);
  ((unsigned char *)values[count].buffer)[val_len] = 0;
  values[count].buffer_length = val_len;
  values[count].buffer_type = MYSQL_TYPE_STRING;
  count++;
  return false;
}


// Source: client_query_attributes.cc
// Lines 33-45
