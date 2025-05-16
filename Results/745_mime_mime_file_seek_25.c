static int mime_file_seek(void *instream, curl_off_t offset, int whence)
{
  curl_mimepart *part = (curl_mimepart *) instream;

  if(whence == SEEK_SET && !offset && !part->fp)
    return CURL_SEEKFUNC_OK;   /* Not open: implicitly already at BOF. */

  if(mime_open_file(part))
    return CURL_SEEKFUNC_FAIL;

  return fseek(part->fp, (long) offset, whence)?
               CURL_SEEKFUNC_CANTSEEK: CURL_SEEKFUNC_OK;
}


// Source: mime.c
// Lines 712-724
