// Source: curl/lib/mime.c
// Lines 1132-1133
static int mime_subparts_seek(void *instream, curl_off_t offset, int whence)
{
  curl_mime *mime = (curl_mime *) instream;
  curl_mimepart *part;
  int result = CURL_SEEKFUNC_OK;

  if(whence != SEEK_SET || offset)
    return CURL_SEEKFUNC_CANTSEEK;    /* Only support full rewind. */

  if(mime->state.state == MIMESTATE_BEGIN)
   return CURL_SEEKFUNC_OK;           /* Already rewound. */

  for(part = mime->firstpart; part; part = part->nextpart) {
    int res = mime_part_rewind(part);
    if(res != CURL_SEEKFUNC_OK)
      result = res;
  }

  if(result == CURL_SEEKFUNC_OK)
    mimesetstate(&mime->state, MIMESTATE_BEGIN, NULL);

  return result;
}
