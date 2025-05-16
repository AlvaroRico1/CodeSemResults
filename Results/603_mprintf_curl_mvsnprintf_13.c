int curl_mvsnprintf(char *buffer, size_t maxlength, const char *format,
                    va_list ap_save)
{
  int retcode;
  struct nsprintf info;

  info.buffer = buffer;
  info.length = 0;
  info.max = maxlength;

  retcode = dprintf_formatf(&info, addbyter, format, ap_save);
  if((retcode != -1) && info.max) {
    /* we terminate this with a zero byte */
    if(info.max == info.length) {
      /* we're at maximum, scrap the last letter */
      info.buffer[-1] = 0;
      retcode--; /* don't count the nul byte */
    }
    else
      info.buffer[0] = 0;
  }
  return retcode;
}


// Source: mprintf.c
// Lines 1007-1029
