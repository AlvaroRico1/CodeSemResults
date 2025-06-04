PREFIX(bcmp_translate) (const CHAR_T *s1, const CHAR_T *s2, register int len,
                        RE_TRANSLATE_TYPE translate)
{
  register const UCHAR_T *p1 = (const UCHAR_T *) s1;
  register const UCHAR_T *p2 = (const UCHAR_T *) s2;
  while (len)
    {
#ifdef WCHAR
      if (((*p1<=0xff)?translate[*p1++]:*p1++)
	  != ((*p2<=0xff)?translate[*p2++]:*p2++))
	return 1;
#else /* BYTE */
      if (translate[*p1++] != translate[*p2++]) return 1;
#endif /* WCHAR */
      len--;
    }
  return 0;
}


// Source: regex.c
// Lines 7739-7756
