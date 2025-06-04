CURLofft curlx_strtoofft(const char *str, char **endp, int base,
                         curl_off_t *num)
{
  char *end;
  curl_off_t number;
  errno = 0;
  *num = 0; /* clear by default */

  while(*str && ISSPACE(*str))
    str++;
  if('-' == *str) {
    if(endp)
      *endp = (char *)str; /* didn't actually move */
    return CURL_OFFT_INVAL; /* nothing parsed */
  }
  number = strtooff(str, &end, base);
  if(endp)
    *endp = end;
  if(errno == ERANGE)
    /* overflow/underflow */
    return CURL_OFFT_FLOW;
  else if(str == end)
    /* nothing parsed */
    return CURL_OFFT_INVAL;

  *num = number;
  return CURL_OFFT_OK;
}


// Source: strtoofft.c
// Lines 215-242
