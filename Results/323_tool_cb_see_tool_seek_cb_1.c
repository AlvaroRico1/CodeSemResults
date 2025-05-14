// Source: curl/src/tool_cb_see.c
// Lines 47-49
int tool_seek_cb(void *userdata, curl_off_t offset, int whence)
{
  struct InStruct *in = userdata;

#if(SIZEOF_CURL_OFF_T > SIZEOF_OFF_T) && !defined(USE_WIN32_LARGE_FILES)

  /* The offset check following here is only interesting if curl_off_t is
     larger than off_t and we are not using the WIN32 large file support
     macros that provide the support to do 64bit seeks correctly */

  if(offset > OUR_MAX_SEEK_O) {
    /* Some precaution code to work around problems with different data sizes
       to allow seeking >32bit even if off_t is 32bit. Should be very rare and
       is really valid on weirdo-systems. */
    curl_off_t left = offset;

    if(whence != SEEK_SET)
      /* this code path doesn't support other types */
      return CURL_SEEKFUNC_FAIL;

    if(LSEEK_ERROR == lseek(in->fd, 0, SEEK_SET))
      /* couldn't rewind to beginning */
      return CURL_SEEKFUNC_FAIL;

    while(left) {
      long step = (left > OUR_MAX_SEEK_O) ? OUR_MAX_SEEK_L : (long)left;
      if(LSEEK_ERROR == lseek(in->fd, step, SEEK_CUR))
        /* couldn't seek forwards the desired amount */
        return CURL_SEEKFUNC_FAIL;
      left -= step;
    }
    return CURL_SEEKFUNC_OK;
  }
#endif

  if(LSEEK_ERROR == lseek(in->fd, offset, whence))
    /* couldn't rewind, the reason is in errno but errno is just not portable
       enough and we don't actually care that much why we failed. We'll let
       libcurl know that it may try other means if it wants to. */
    return CURL_SEEKFUNC_CANTSEEK;

  return CURL_SEEKFUNC_OK;
}
