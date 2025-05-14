// Source: curl/src/tool_formparse.c
// Lines 219-221
int tool_mime_stdin_seek(void *instream, curl_off_t offset, int whence)
{
  struct tool_mime *sip = (struct tool_mime *) instream;

  switch(whence) {
  case SEEK_CUR:
    offset += sip->curpos;
    break;
  case SEEK_END:
    offset += sip->size;
    break;
  }
  if(offset < 0)
    return CURL_SEEKFUNC_CANTSEEK;
  if(!sip->data) {
    if(fseek(stdin, (long) (offset + sip->origin), SEEK_SET))
      return CURL_SEEKFUNC_CANTSEEK;
  }
  sip->curpos = offset;
  return CURL_SEEKFUNC_OK;
}
