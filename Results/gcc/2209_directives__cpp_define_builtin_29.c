_cpp_define_builtin (cpp_reader *pfile, const char *str)
{
  size_t len = strlen (str);
  char *buf = (char *) alloca (len + 1);
  memcpy (buf, str, len);
  buf[len] = '\n';
  run_directive (pfile, T_DEFINE, buf, len);
}


// Source: directives.c
// Lines 2424-2431
