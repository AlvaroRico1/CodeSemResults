// Source: curl/src/tool_formparse.c
// Lines 243-249
static CURLcode tool2curlparts(CURL *curl, struct tool_mime *m,
                               curl_mime *mime)
{
  CURLcode ret = CURLE_OK;
  curl_mimepart *part = NULL;
  curl_mime *submime = NULL;
  const char *filename = NULL;

  if(m) {
    ret = tool2curlparts(curl, m->prev, mime);
    if(!ret) {
      part = curl_mime_addpart(mime);
      if(!part)
        ret = CURLE_OUT_OF_MEMORY;
    }
    if(!ret) {
      filename = m->filename;
      switch(m->kind) {
      case TOOLMIME_PARTS:
        ret = tool2curlmime(curl, m, &submime);
        if(!ret) {
          ret = curl_mime_subparts(part, submime);
          if(ret)
            curl_mime_free(submime);
        }
        break;

      case TOOLMIME_DATA:
#ifdef CURL_DOES_CONVERSIONS
        /* Our data is always textual: convert it to ASCII. */
        {
          size_t size = strlen(m->data);
          char *cp = malloc(size + 1);

          if(!cp)
            ret = CURLE_OUT_OF_MEMORY;
          else {
            memcpy(cp, m->data, size + 1);
            ret = convert_to_network(cp, size);
            if(!ret)
              ret = curl_mime_data(part, cp, CURL_ZERO_TERMINATED);
            free(cp);
          }
        }
#else
        ret = curl_mime_data(part, m->data, CURL_ZERO_TERMINATED);
#endif
        break;

      case TOOLMIME_FILE:
      case TOOLMIME_FILEDATA:
        ret = curl_mime_filedata(part, m->data);
        if(!ret && m->kind == TOOLMIME_FILEDATA && !filename)
          ret = curl_mime_filename(part, NULL);
        break;

      case TOOLMIME_STDIN:
        if(!filename)
          filename = "-";
        /* FALLTHROUGH */
      case TOOLMIME_STDINDATA:
        ret = curl_mime_data_cb(part, m->size,
                                (curl_read_callback) tool_mime_stdin_read,
                                (curl_seek_callback) tool_mime_stdin_seek,
                                NULL, m);
        break;

      default:
        /* Other cases not possible in this context. */
        break;
      }
    }
    if(!ret && filename)
      ret = curl_mime_filename(part, filename);
    if(!ret)
      ret = curl_mime_type(part, m->type);
    if(!ret)
      ret = curl_mime_headers(part, m->headers, 0);
    if(!ret)
      ret = curl_mime_encoder(part, m->encoder);
    if(!ret)
      ret = curl_mime_name(part, m->name);
  }
  return ret;
}
