append_file_to_dir (const char *fname, cpp_dir *dir)
{
  size_t dlen, flen;
  char *path;

  dlen = dir->len;
  flen = strlen (fname);
  path = XNEWVEC (char, dlen + 1 + flen + 1);
  memcpy (path, dir->name, dlen);
  if (dlen && !IS_DIR_SEPARATOR (path[dlen - 1]))
    path[dlen++] = '/';
  memcpy (&path[dlen], fname, flen + 1);

  return path;
}


// Source: files.c
// Lines 1560-1574
