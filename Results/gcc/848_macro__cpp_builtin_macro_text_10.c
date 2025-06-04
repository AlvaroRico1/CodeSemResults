_cpp_builtin_macro_text (cpp_reader *pfile, cpp_hashnode *node,
			 location_t loc)
{
  const uchar *result = NULL;
  linenum_type number = 1;

  switch (node->value.builtin)
    {
    default:
      cpp_error (pfile, CPP_DL_ICE, "invalid built-in macro \"%s\"",
		 NODE_NAME (node));
      break;

    case BT_TIMESTAMP:
      {
	if (CPP_OPTION (pfile, warn_date_time))
	  cpp_warning (pfile, CPP_W_DATE_TIME, "macro \"%s\" might prevent "
		       "reproducible builds", NODE_NAME (node));

	cpp_buffer *pbuffer = cpp_get_buffer (pfile);
	if (pbuffer->timestamp == NULL)
	  {
	    /* Initialize timestamp value of the assotiated file. */
            struct _cpp_file *file = cpp_get_file (pbuffer);
	    if (file)
	      {
    		/* Generate __TIMESTAMP__ string, that represents 
		   the date and time of the last modification 
		   of the current source file. The string constant 
		   looks like "Sun Sep 16 01:03:52 1973".  */
		struct tm *tb = NULL;
		struct stat *st = _cpp_get_file_stat (file);
		if (st)
		  tb = localtime (&st->st_mtime);
		if (tb)
		  {
		    char *str = asctime (tb);
		    size_t len = strlen (str);
		    unsigned char *buf = _cpp_unaligned_alloc (pfile, len + 2);
		    buf[0] = '"';
		    strcpy ((char *) buf + 1, str);
		    buf[len] = '"';
		    pbuffer->timestamp = buf;
		  }


// Source: macro.c
// Lines 480-523
