// Source: curl/lib/mime.c
// Lines 642-644
static size_t mime_mem_read(char *buffer, size_t size, size_t nitems,
                            void *instream)
{
  curl_mimepart *part = (curl_mimepart *) instream;
  size_t sz = curlx_sotouz(part->datasize - part->state.offset);
  (void) size;   /* Always 1.*/

  if(!nitems)
    return STOP_FILLING;

  if(sz > nitems)
    sz = nitems;

  if(sz)
    memcpy(buffer, part->data + curlx_sotouz(part->state.offset), sz);

  return sz;
}
