CURLcode Curl_build_unencoding_stack(struct Curl_easy *data,
                                     const char *enclist, int maybechunked)
{
  struct SingleRequest *k = &data->req;

  do {
    const char *name;
    size_t namelen;

    /* Parse a single encoding name. */
    while(ISSPACE(*enclist) || *enclist == ',')
      enclist++;

    name = enclist;

    for(namelen = 0; *enclist && *enclist != ','; enclist++)
      if(!ISSPACE(*enclist))
        namelen = enclist - name + 1;

    /* Special case: chunked encoding is handled at the reader level. */
    if(maybechunked && namelen == 7 && strncasecompare(name, "chunked", 7)) {
      k->chunk = TRUE;             /* chunks coming our way. */
      Curl_httpchunk_init(data);   /* init our chunky engine. */
    }
    else if(namelen) {
      const struct content_encoding *encoding = find_encoding(name, namelen);
      struct contenc_writer *writer;

      if(!k->writer_stack) {
        k->writer_stack = new_unencoding_writer(data, &client_encoding, NULL);

        if(!k->writer_stack)
          return CURLE_OUT_OF_MEMORY;
      }

      if(!encoding)
        encoding = &error_encoding;  /* Defer error at stack use. */

      /* Stack the unencoding stage. */
      writer = new_unencoding_writer(data, encoding, k->writer_stack);
      if(!writer)
        return CURLE_OUT_OF_MEMORY;
      k->writer_stack = writer;
    }
  } while(*enclist);


// Source: content_encoding.c
// Lines 1030-1074
