cpp_undef (cpp_reader *pfile, const char *macro)
{
  size_t len = strlen (macro);
  char *buf = (char *) alloca (len + 1);
  memcpy (buf, macro, len);
  buf[len] = '\n';
  run_directive (pfile, T_UNDEF, buf, len);
}


// Source: directives.c
// Lines 2435-2442
