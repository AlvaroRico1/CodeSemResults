void STDCALL myodbc_remove_escape(MYSQL *mysql, char *name) {
  char *to;
  bool use_mb_flag = use_mb(mysql->charset);
  char *end = nullptr;
  if (use_mb_flag)
    for (end = name; *end; end++)
      ;

  for (to = name; *name; name++) {
    int l;
    if (use_mb_flag && (l = my_ismbchar(mysql->charset, name, end))) {
      while (l--) *to++ = *name++;
      name--;
      continue;
    }
    if (*name == '\\' && name[1]) name++;
    *to++ = *name;
  }


// Source: libmysql.cc
// Lines 1130-1147
