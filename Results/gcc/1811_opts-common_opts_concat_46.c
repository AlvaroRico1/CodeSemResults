opts_concat (const char *first, ...)
{
  char *newstr, *end;
  size_t length = 0;
  const char *arg;
  va_list ap;

  /* First compute the size of the result and get sufficient memory.  */
  va_start (ap, first);
  for (arg = first; arg; arg = va_arg (ap, const char *))
    length += strlen (arg);
  newstr = XOBNEWVEC (&opts_obstack, char, length + 1);
  va_end (ap);

  /* Now copy the individual pieces to the result string. */
  va_start (ap, first);
  for (arg = first, end = newstr; arg; arg = va_arg (ap, const char *))
    {
      length = strlen (arg);
      memcpy (end, arg, length);
      end += length;
    }
  *end = '\0';
  va_end (ap);
  return newstr;
}


// Source: opts-common.c
// Lines 901-926
