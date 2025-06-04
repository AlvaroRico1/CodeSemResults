hash_rtx_string (const char *ps)
{
  unsigned hash = 0;
  const unsigned char *p = (const unsigned char *) ps;

  if (p)
    while (*p)
      hash += *p++;

  return hash;
}


// Source: cse.c
// Lines 2212-2222
