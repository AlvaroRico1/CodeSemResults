parse_include (cpp_reader *pfile, int *pangle_brackets,
	       const cpp_token ***buf, location_t *location)
{
  char *fname;
  const cpp_token *header;

  /* Allow macro expansion.  */
  header = get_token_no_padding (pfile);
  *location = header->src_loc;
  if ((header->type == CPP_STRING && header->val.str.text[0] != 'R')
      || header->type == CPP_HEADER_NAME)
    {
      fname = XNEWVEC (char, header->val.str.len - 1);
      memcpy (fname, header->val.str.text + 1, header->val.str.len - 2);
      fname[header->val.str.len - 2] = '\0';
      *pangle_brackets = header->type == CPP_HEADER_NAME;
    }
  else if (header->type == CPP_LESS)
    {
      fname = glue_header_name (pfile);
      *pangle_brackets = 1;
    }
  else
    {
      const unsigned char *dir;

      if (pfile->directive == &dtable[T_PRAGMA])
	dir = UC"pragma dependency";
      else
	dir = pfile->directive->name;
      cpp_error (pfile, CPP_DL_ERROR, "#%s expects \"FILENAME\" or <FILENAME>",
		 dir);

      return NULL;
    }

  if (pfile->directive == &dtable[T_PRAGMA])
    {
      /* This pragma allows extra tokens after the file name.  */
    }
  else if (buf == NULL || CPP_OPTION (pfile, discard_comments))
    check_eol (pfile, true);
  else
    {
      /* If we are not discarding comments, then gather them while
	 doing the eol check.  */
      *buf = check_eol_return_comments (pfile);
    }

  return fname;
}


// Source: directives.c
// Lines 753-803
