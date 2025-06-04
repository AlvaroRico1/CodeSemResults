make_temp_file_with_prefix (const char *prefix, const char *suffix)
{
  const char *base = choose_tmpdir ();
  char *temp_filename;
  int base_len, suffix_len, prefix_len;
  int fd;

  if (prefix == 0)
    prefix = "cc";

  if (suffix == 0)
    suffix = "";

  base_len = strlen (base);
  prefix_len = strlen (prefix);
  suffix_len = strlen (suffix);

  temp_filename = XNEWVEC (char, base_len
			   + TEMP_FILE_LEN
			   + suffix_len
			   + prefix_len + 1);
  strcpy (temp_filename, base);
  strcpy (temp_filename + base_len, prefix);
  strcpy (temp_filename + base_len + prefix_len, TEMP_FILE);
  strcpy (temp_filename + base_len + prefix_len + TEMP_FILE_LEN, suffix);

  fd = mkstemps (temp_filename, suffix_len);
  /* Mkstemps failed.  It may be EPERM, ENOSPC etc.  */
  if (fd == -1)
    {
      fprintf (stderr, "Cannot create temporary file in %s: %s\n",
	       base, strerror (errno));
      abort ();
    }
  /* We abort on failed close out of sheer paranoia.  */
  if (close (fd))
    abort ();
  return temp_filename;
}


// Source: make-temp-file.c
// Lines 184-222
