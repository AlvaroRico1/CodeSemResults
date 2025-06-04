CURLcode Curl_getformdata(struct Curl_easy *data,
                          curl_mimepart *finalform,
                          struct curl_httppost *post,
                          curl_read_callback fread_func)
{
  CURLcode result = CURLE_OK;
  curl_mime *form = NULL;
  curl_mimepart *part;
  struct curl_httppost *file;

  Curl_mime_cleanpart(finalform); /* default form is empty */

  if(!post)
    return result; /* no input => no output! */

  form = curl_mime_init(data);
  if(!form)
    result = CURLE_OUT_OF_MEMORY;

  if(!result)
    result = curl_mime_subparts(finalform, form);

  /* Process each top part. */
  for(; !result && post; post = post->next) {
    /* If we have more than a file here, create a mime subpart and fill it. */
    curl_mime *multipart = form;
    if(post->more) {
      part = curl_mime_addpart(form);
      if(!part)
        result = CURLE_OUT_OF_MEMORY;
      if(!result)
        result = setname(part, post->name, post->namelength);
      if(!result) {
        multipart = curl_mime_init(data);
        if(!multipart)
          result = CURLE_OUT_OF_MEMORY;
      }
      if(!result)
        result = curl_mime_subparts(part, multipart);
    }

    /* Generate all the part contents. */
    for(file = post; !result && file; file = file->more) {
      /* Create the part. */
      part = curl_mime_addpart(multipart);
      if(!part)
        result = CURLE_OUT_OF_MEMORY;

      /* Set the headers. */
      if(!result)
        result = curl_mime_headers(part, file->contentheader, 0);

      /* Set the content type. */
      if(!result && file->contenttype)
        result = curl_mime_type(part, file->contenttype);

      /* Set field name. */
      if(!result && !post->more)
        result = setname(part, post->name, post->namelength);

      /* Process contents. */
      if(!result) {
        curl_off_t clen = post->contentslength;

        if(post->flags & CURL_HTTPPOST_LARGE)
          clen = post->contentlen;

        if(post->flags & (HTTPPOST_FILENAME | HTTPPOST_READFILE)) {
          if(!strcmp(file->contents, "-")) {
            /* There are a few cases where the code below won't work; in
               particular, freopen(stdin) by the caller is not guaranteed
               to result as expected. This feature has been kept for backward
               compatibility: use of "-" pseudo file name should be avoided. */
            result = curl_mime_data_cb(part, (curl_off_t) -1,
                                       (curl_read_callback) fread,
                                       CURLX_FUNCTION_CAST(curl_seek_callback,
                                                           fseek),
                                       NULL, (void *) stdin);
          }
          else
            result = curl_mime_filedata(part, file->contents);
          if(!result && (post->flags & HTTPPOST_READFILE))
            result = curl_mime_filename(part, NULL);
        }
        else if(post->flags & HTTPPOST_BUFFER)
          result = curl_mime_data(part, post->buffer,
                                  post->bufferlength? post->bufferlength: -1);
        else if(post->flags & HTTPPOST_CALLBACK) {
          /* the contents should be read with the callback and the size is set
             with the contentslength */
          if(!clen)
            clen = -1;
          result = curl_mime_data_cb(part, clen,
                                     fread_func, NULL, NULL, post->userp);
        }
        else {
          size_t uclen;
          if(!clen)
            uclen = CURL_ZERO_TERMINATED;
          else
            uclen = (size_t)clen;
          result = curl_mime_data(part, post->contents, uclen);
#ifdef CURL_DOES_CONVERSIONS
          /* Convert textual contents now. */
          if(!result && data && part->datasize)
            result = Curl_convert_to_network(data, part->data, part->datasize);
#endif
        }
      }

      /* Set fake file name. */
      if(!result && post->showfilename)
        if(post->more || (post->flags & (HTTPPOST_FILENAME | HTTPPOST_BUFFER |
                                        HTTPPOST_CALLBACK)))
          result = curl_mime_filename(part, post->showfilename);
    }
  }

  if(result)
    Curl_mime_cleanpart(finalform);

  return result;
}


// Source: formdata.c
// Lines 802-924
