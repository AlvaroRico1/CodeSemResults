stringify_arg (cpp_reader *pfile, macro_arg *arg)
{
  unsigned char *dest;
  unsigned int i, escape_it, backslash_count = 0;
  const cpp_token *source = NULL;
  size_t len;

  if (BUFF_ROOM (pfile->u_buff) < 3)
    _cpp_extend_buff (pfile, &pfile->u_buff, 3);
  dest = BUFF_FRONT (pfile->u_buff);
  *dest++ = '"';

  /* Loop, reading in the argument's tokens.  */
  for (i = 0; i < arg->count; i++)
    {
      const cpp_token *token = arg->first[i];

      if (token->type == CPP_PADDING)
	{
	  if (source == NULL
	      || (!(source->flags & PREV_WHITE)
		  && token->val.source == NULL))
	    source = token->val.source;
	  continue;
	}

      escape_it = (token->type == CPP_STRING || token->type == CPP_CHAR
		   || token->type == CPP_WSTRING || token->type == CPP_WCHAR
		   || token->type == CPP_STRING32 || token->type == CPP_CHAR32
		   || token->type == CPP_STRING16 || token->type == CPP_CHAR16
		   || token->type == CPP_UTF8STRING || token->type == CPP_UTF8CHAR
		   || cpp_userdef_string_p (token->type)
		   || cpp_userdef_char_p (token->type));

      /* Room for each char being written in octal, initial space and
	 final quote and NUL.  */
      len = cpp_token_len (token);
      if (escape_it)
	len *= 4;
      len += 3;

      if ((size_t) (BUFF_LIMIT (pfile->u_buff) - dest) < len)
	{
	  size_t len_so_far = dest - BUFF_FRONT (pfile->u_buff);
	  _cpp_extend_buff (pfile, &pfile->u_buff, len);
	  dest = BUFF_FRONT (pfile->u_buff) + len_so_far;
	}

      /* Leading white space?  */
      if (dest - 1 != BUFF_FRONT (pfile->u_buff))
	{
	  if (source == NULL)
	    source = token;
	  if (source->flags & PREV_WHITE)
	    *dest++ = ' ';
	}
      source = NULL;

      if (escape_it)
	{
	  _cpp_buff *buff = _cpp_get_buff (pfile, len);
	  unsigned char *buf = BUFF_FRONT (buff);
	  len = cpp_spell_token (pfile, token, buf, true) - buf;
	  dest = cpp_quote_string (dest, buf, len);
	  _cpp_release_buff (pfile, buff);
	}


// Source: macro.c
// Lines 787-852
