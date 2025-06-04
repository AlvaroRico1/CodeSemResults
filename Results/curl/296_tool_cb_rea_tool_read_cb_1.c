size_t tool_read_cb(char *buffer, size_t sz, size_t nmemb, void *userdata)
{
  ssize_t rc;
  struct InStruct *in = userdata;

  rc = read(in->fd, buffer, sz*nmemb);
  if(rc < 0) {
    if(errno == EAGAIN) {
      errno = 0;
      in->config->readbusy = TRUE;
      return CURL_READFUNC_PAUSE;
    }
    /* since size_t is unsigned we can't return negative values fine */
    rc = 0;
  }
  in->config->readbusy = FALSE;
  return (size_t)rc;
}


// Source: tool_cb_rea.c
// Lines 38-55
