static size_t readmoredata(char *buffer,
                           size_t size,
                           size_t nitems,
                           void *userp)
{
  struct Curl_easy *data = (struct Curl_easy *)userp;
  struct HTTP *http = data->req.p.http;
  size_t fullsize = size * nitems;

  if(!http->postsize)
    /* nothing to return */
    return 0;

  /* make sure that a HTTP request is never sent away chunked! */
  data->req.forbidchunk = (http->sending == HTTPSEND_REQUEST)?TRUE:FALSE;

  if(data->set.max_send_speed &&
     (data->set.max_send_speed < (curl_off_t)fullsize) &&
     (data->set.max_send_speed < http->postsize))
    /* speed limit */
    fullsize = (size_t)data->set.max_send_speed;

  else if(http->postsize <= (curl_off_t)fullsize) {
    memcpy(buffer, http->postdata, (size_t)http->postsize);
    fullsize = (size_t)http->postsize;

    if(http->backup.postsize) {
      /* move backup data into focus and continue on that */
      http->postdata = http->backup.postdata;
      http->postsize = http->backup.postsize;
      data->state.fread_func = http->backup.fread_func;
      data->state.in = http->backup.fread_in;

      http->sending++; /* move one step up */

      http->backup.postsize = 0;
    }
    else
      http->postsize = 0;

    return fullsize;
  }

  memcpy(buffer, http->postdata, fullsize);
  http->postdata += fullsize;
  http->postsize -= fullsize;

  return fullsize;
}


// Source: http.c
// Lines 1165-1213
