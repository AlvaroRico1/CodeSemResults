get_spaces (const char *str)
{
   size_t len = gcc_gettext_width (str);
   char *spaces = XNEWVEC (char, len + 1);
   memset (spaces, ' ', len);
   spaces[len] = '\0';
   return spaces;
}


// Source: intl.c
// Lines 140-147
