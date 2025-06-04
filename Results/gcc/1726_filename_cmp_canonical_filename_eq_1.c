canonical_filename_eq (const char * a, const char * b)
{
  char * ca = lrealpath(a);
  char * cb = lrealpath(b);
  int res = filename_eq (ca, cb);
  free (ca);
  free (cb);
  return res;
}


// Source: filename_cmp.c
// Lines 213-221
