// Source: curl/lib/mime.c
// Lines 699-701
static size_t mime_file_read(char *buffer, size_t size, size_t nitems,
                             void *instream)
{
  curl_mimepart *part = (curl_mimepart *) instream;

  if(!nitems)
    return STOP_FILLING;

  if(mime_open_file(part))
    return READ_ERROR;

  return fread(buffer, size, nitems, part->fp);
}
