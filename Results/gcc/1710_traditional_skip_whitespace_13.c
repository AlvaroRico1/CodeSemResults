skip_whitespace (cpp_reader *pfile, const uchar *cur, int skip_comments)
{
  uchar *out = pfile->out.cur;

  for (;;)
    {
      unsigned int c = *cur++;
      *out++ = c;

      if (is_nvspace (c))
	continue;

      if (c == '/' && *cur == '*' && skip_comments)
	{
	  pfile->out.cur = out;
	  cur = copy_comment (pfile, cur, false /* in_define */);
	  out = pfile->out.cur;
	  continue;
	}

      out--;
      break;
    }

  pfile->out.cur = out;
  return cur - 1;
}


// Source: traditional.c
// Lines 224-250
