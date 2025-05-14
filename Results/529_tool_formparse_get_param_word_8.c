// Source: curl/src/tool_formparse.c
// Lines 350-356
static char *get_param_word(struct OperationConfig *config, char **str,
                            char **end_pos, char endchar)
{
  char *ptr = *str;
  /* the first non-space char is here */
  char *word_begin = ptr;
  char *ptr2;
  char *escape = NULL;

  if(*ptr == '"') {
    ++ptr;
    while(*ptr) {
      if(*ptr == '\\') {
        if(ptr[1] == '\\' || ptr[1] == '"') {
          /* remember the first escape position */
          if(!escape)
            escape = ptr;
          /* skip escape of back-slash or double-quote */
          ptr += 2;
          continue;
        }
      }
      if(*ptr == '"') {
        bool trailing_data = FALSE;
        *end_pos = ptr;
        if(escape) {
          /* has escape, we restore the unescaped string here */
          ptr = ptr2 = escape;
          do {
            if(*ptr == '\\' && (ptr[1] == '\\' || ptr[1] == '"'))
              ++ptr;
            *ptr2++ = *ptr++;
          }
          while(ptr < *end_pos);
          *end_pos = ptr2;
        }
        ++ptr;
        while(*ptr && *ptr != ';' && *ptr != endchar) {
          if(!ISSPACE(*ptr))
            trailing_data = TRUE;
          ++ptr;
        }
        if(trailing_data)
          warnf(config->global, "Trailing data after quoted form parameter\n");
        *str = ptr;
        return word_begin + 1;
      }
      ++ptr;
    }
    /* end quote is missing, treat it as non-quoted. */
    ptr = word_begin;
  }

  while(*ptr && *ptr != ';' && *ptr != endchar)
    ++ptr;
  *str = *end_pos = ptr;
  return word_begin;
}
