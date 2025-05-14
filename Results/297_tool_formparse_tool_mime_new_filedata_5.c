// Source: curl/src/tool_formparse.c
// Lines 83-88
static struct tool_mime *tool_mime_new_filedata(struct tool_mime *parent,
                                                const char *filename,
                                                bool isremotefile,
                                                CURLcode *errcode)
{
  CURLcode result = CURLE_OK;
  struct tool_mime *m = NULL;

  *errcode = CURLE_OUT_OF_MEMORY;
  if(strcmp(filename, "-")) {
    /* This is a normal file. */
    filename = strdup(filename);
    if(filename) {
      m = tool_mime_new(parent, TOOLMIME_FILE);
      if(!m)
        CONST_FREE(filename);
      else {
        m->data = filename;
        if(!isremotefile)
          m->kind = TOOLMIME_FILEDATA;
       *errcode = CURLE_OK;
      }
    }
  }
  else {        /* Standard input. */
    int fd = fileno(stdin);
    char *data = NULL;
    curl_off_t size;
    curl_off_t origin;
    struct_stat sbuf;

    set_binmode(stdin);
    origin = ftell(stdin);
    /* If stdin is a regular file, do not buffer data but read it
       when needed. */
    if(fd >= 0 && origin >= 0 && !fstat(fd, &sbuf) &&
#ifdef __VMS
       sbuf.st_fab_rfm != FAB$C_VAR && sbuf.st_fab_rfm != FAB$C_VFC &&
#endif
       S_ISREG(sbuf.st_mode)) {
      size = sbuf.st_size - origin;
      if(size < 0)
        size = 0;
    }
    else {  /* Not suitable for direct use, buffer stdin data. */
      size_t stdinsize = 0;

      if(file2memory(&data, &stdinsize, stdin) != PARAM_OK) {
        /* Out of memory. */
        return m;
      }

      if(ferror(stdin)) {
        result = CURLE_READ_ERROR;
        Curl_safefree(data);
        data = NULL;
      }
      else if(!stdinsize) {
        /* Zero-length data has been freed. Re-create it. */
        data = strdup("");
        if(!data)
          return m;
      }
      size = curlx_uztoso(stdinsize);
      origin = 0;
    }
    m = tool_mime_new(parent, TOOLMIME_STDIN);
    if(!m)
      Curl_safefree(data);
    else {
      m->data = data;
      m->origin = origin;
      m->size = size;
      m->curpos = 0;
      if(!isremotefile)
        m->kind = TOOLMIME_STDINDATA;
      *errcode = result;
    }
  }
  return m;
}
