elf_find_debugfile_by_debuglink (struct backtrace_state *state,
				 const char *filename,
				 const char *debuglink_name,
				 backtrace_error_callback error_callback,
				 void *data)
{
  int ret;
  char *alc;
  size_t alc_len;
  const char *slash;
  int ddescriptor;
  const char *prefix;
  size_t prefix_len;

  /* Resolve symlinks in FILENAME.  Since FILENAME is fairly likely to
     be /proc/self/exe, symlinks are common.  We don't try to resolve
     the whole path name, just the base name.  */
  ret = -1;
  alc = NULL;
  alc_len = 0;
  while (elf_is_symlink (filename))
    {
      char *new_buf;
      size_t new_len;

      new_buf = elf_readlink (state, filename, error_callback, data, &new_len);
      if (new_buf == NULL)
	break;

      if (new_buf[0] == '/')
	filename = new_buf;
      else
	{
	  slash = strrchr (filename, '/');
	  if (slash == NULL)
	    filename = new_buf;
	  else
	    {
	      size_t clen;
	      char *c;

	      slash++;
	      clen = slash - filename + strlen (new_buf) + 1;
	      c = backtrace_alloc (state, clen, error_callback, data);
	      if (c == NULL)
		goto done;

	      memcpy (c, filename, slash - filename);
	      memcpy (c + (slash - filename), new_buf, strlen (new_buf));
	      c[slash - filename + strlen (new_buf)] = '\0';
	      backtrace_free (state, new_buf, new_len, error_callback, data);
	      filename = c;
	      new_buf = c;
	      new_len = clen;
	    }
	}

      if (alc != NULL)
	backtrace_free (state, alc, alc_len, error_callback, data);
      alc = new_buf;
      alc_len = new_len;
    }


// Source: elf.c
// Lines 888-949
