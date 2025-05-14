// Source: curl/lib/mime.c
// Lines 1622-1624
size_t Curl_mime_read(char *buffer, size_t size, size_t nitems, void *instream)
{
  curl_mimepart *part = (curl_mimepart *) instream;
  size_t ret;
  bool hasread;

  (void) size;   /* Always 1. */

  do {
    hasread = FALSE;
    ret = readback_part(part, buffer, nitems, &hasread);
    /*
     * If this is not possible to get some data without calling more than
     * one read callback (probably because a content encoder is not able to
     * deliver a new bunch for the few data accumulated so far), force another
     * read until we get enough data or a special exit code.
     */
  } while(ret == STOP_FILLING);

  return ret;
}
