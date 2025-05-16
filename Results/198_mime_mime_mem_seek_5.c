static int mime_mem_seek(void *instream, curl_off_t offset, int whence)
{
  curl_mimepart *part = (curl_mimepart *) instream;

  switch(whence) {
  case SEEK_CUR:
    offset += part->state.offset;
    break;
  case SEEK_END:
    offset += part->datasize;
    break;
  }

  if(offset < 0 || offset > part->datasize)
    return CURL_SEEKFUNC_FAIL;

  part->state.offset = offset;
  return CURL_SEEKFUNC_OK;
}


// Source: mime.c
// Lines 660-678
