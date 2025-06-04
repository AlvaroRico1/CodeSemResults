cpp_define (cpp_reader *pfile, const char *str)
{
  char *buf;
  const char *p;
  size_t count;

  /* Copy the entire option so we can modify it.
     Change the first "=" in the string to a space.  If there is none,
     tack " 1" on the end.  */

  count = strlen (str);
  buf = (char *) alloca (count + 3);
  memcpy (buf, str, count);

  p = strchr (str, '=');
  if (p)
    buf[p - str] = ' ';
  else
    {
      buf[count++] = ' ';
      buf[count++] = '1';
    }
  buf[count] = '\n';

  run_directive (pfile, T_DEFINE, buf, count);
}


// Source: directives.c
// Lines 2375-2400
