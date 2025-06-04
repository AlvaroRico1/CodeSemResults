static inline void convert_dash_to_underscore(char *str, size_t len) {
  for (char *p = str; p <= str + len; p++)
    if (*p == '-') *p = '_';
}


// Source: sql_plugin.cc
// Lines 1337-1340
