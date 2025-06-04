trim_filename (const char *name)
{
  static const char this_file[] = __FILE__;
  const char *p = name, *q = this_file;

  /* First skip any "../" in each filename.  This allows us to give a proper
     reference to a file in a subdirectory.  */
  while (p[0] == '.' && p[1] == '.' && IS_DIR_SEPARATOR (p[2]))
    p += 3;

  while (q[0] == '.' && q[1] == '.' && IS_DIR_SEPARATOR (q[2]))
    q += 3;

  /* Now skip any parts the two filenames have in common.  */
  while (*p == *q && *p != 0 && *q != 0)
    p++, q++;

  /* Now go backwards until the previous directory separator.  */
  while (p > name && !IS_DIR_SEPARATOR (p[-1]))
    p--;

  return p;
}


// Source: diagnostic.c
// Lines 1213-1235
