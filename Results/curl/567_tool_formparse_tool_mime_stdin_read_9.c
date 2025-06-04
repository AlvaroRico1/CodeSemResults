size_t tool_mime_stdin_read(char *buffer,
                            size_t size, size_t nitems, void *arg)
{
  struct tool_mime *sip = (struct tool_mime *) arg;
  curl_off_t bytesleft;
  (void) size;  /* Always 1: ignored. */

  if(sip->size >= 0) {
    if(sip->curpos >= sip->size)
      return 0;  /* At eof. */
    bytesleft = sip->size - sip->curpos;
    if(curlx_uztoso(nitems) > bytesleft)
      nitems = curlx_sotouz(bytesleft);
  }
  if(nitems) {
    if(sip->data) {
      /* Return data from memory. */
      memcpy(buffer, sip->data + curlx_sotouz(sip->curpos), nitems);
    }
    else {
      /* Read from stdin. */
      nitems = fread(buffer, 1, nitems, stdin);
      if(ferror(stdin)) {
        /* Show error only once. */
        if(sip->config) {
          warnf(sip->config, "stdin: %s\n", strerror(errno));
          sip->config = NULL;
        }
        return CURL_READFUNC_ABORT;
      }
    }
    sip->curpos += curlx_uztoso(nitems);
  }
  return nitems;
}


// Source: tool_formparse.c
// Lines 183-217
