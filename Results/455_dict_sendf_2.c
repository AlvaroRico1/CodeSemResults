// Source: curl/lib/dict.c
// Lines 139-140
static CURLcode sendf(curl_socket_t sockfd, struct Curl_easy *data,
                      const char *fmt, ...)
{
  ssize_t bytes_written;
  size_t write_len;
  CURLcode result = CURLE_OK;
  char *s;
  char *sptr;
  va_list ap;
  va_start(ap, fmt);
  s = vaprintf(fmt, ap); /* returns an allocated string */
  va_end(ap);
  if(!s)
    return CURLE_OUT_OF_MEMORY; /* failure */

  bytes_written = 0;
  write_len = strlen(s);
  sptr = s;

  for(;;) {
    /* Write the buffer to the socket */
    result = Curl_write(data, sockfd, sptr, write_len, &bytes_written);

    if(result)
      break;

    Curl_debug(data, CURLINFO_DATA_OUT, sptr, (size_t)bytes_written);

    if((size_t)bytes_written != write_len) {
      /* if not all was written at once, we must advance the pointer, decrease
         the size left and try again! */
      write_len -= bytes_written;
      sptr += bytes_written;
    }
    else
      break;
  }

  free(s); /* free the output string */

  return result;
}
